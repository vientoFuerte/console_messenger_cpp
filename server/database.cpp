#include "database.h"


// Хранилище всех подключённых клиентов (сокеты)
std::vector<clientInfo> clients;
std::mutex clients_mutex;

// Функция для выполнения SQL-запросов
bool execute_sql(const std::string& sql, sqlite3* db) {
    char* errMsg = nullptr;
   // sqlite3_exec -функция для выполнения SQL-запросов, которые не возвращают данные (CREATE, INSERT, UPDATE, DELETE).
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

sqlite3* database_init() {
    sqlite3* db = nullptr;
    int rc = sqlite3_open("messenger.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return nullptr;
    }
    std::cout << "Database opened successfully" << std::endl;
    
    // Создаем таблицу для сообщений
    const char* create_table_sql = 
        "CREATE TABLE IF NOT EXISTS messages ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "from_user TEXT NOT NULL,"
        "to_user TEXT,"
        "message TEXT NOT NULL,"
        "is_private INTEGER DEFAULT 0);";
        
    if (!execute_sql(create_table_sql, db)) {
        std::cerr << "Failed to create table" << std::endl;
        sqlite3_close(db);
        return nullptr;
    }
    
    return db;
}



// Функция для обработки одного клиента
void handle_client(std::shared_ptr<tcp::socket> socket) {
   
   char data[128];
   boost::system::error_code error;
   clientInfo client;
   
    // читаем имя клиента
    size_t len = socket->read_some(boost::asio::buffer(data), error);
    
    if (error) {
        std::cout << "Error reading username: " << error.message() << std::endl;
        return;
    }
    //убираем последний символ перевода строки
    std::string username(data, len -1);
    
    std::cout << "User '" << username << "' connected!" << std::endl;
   
   //  Добавляем клиента в общий список
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.push_back({socket, username});
        std::cout << "Total clients: " << clients.size() << std::endl;
    }   
    while (true) {
        
        size_t len = socket->read_some(boost::asio::buffer(data), error);
        
        if (error == boost::asio::error::eof) {
            std::cout << "Client disconnected!" << std::endl;
            break;
        }
        else if (error) {
            std::cout << "Error: " << error.message() << std::endl;
            break;
        }
  
        std::string message(data, len);
        //std::cout << "Client: " << message << std::endl;
        std::shared_ptr<tcp::socket> target_socket = nullptr;
   
        // Формат сообщения: "@username message"
        if (message[0] == '@') {    
          size_t space_pos = message.find(' ');
          
          if (space_pos != std::string::npos && space_pos > 1) {
            std::string target_username = message.substr(1, space_pos - 1);
            std::string private_message = message.substr(space_pos + 1);
           
            // Ищем получателя
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                
                for(const auto& client: clients) {
                  if(client.username == target_username) {
                      target_socket = client.socket;
                      break;
                  }
                }
                
                if(target_socket)
                {
                    // Отправляем ответ
                    std::string response = "[" + username + "]:" + private_message;
                    boost::asio::write(*target_socket, boost::asio::buffer(response));
                
                }
              
            }

          }
        }
        else if (message == "q" || message == "quit")
        {
            {
                std::lock_guard<std::mutex> lock(clients_mutex);

                auto iter {clients.begin()};
                while(iter != clients.end())
                {
                    if(iter->username == username)
                    {
                        clients.erase(iter);
                        std::cout << "Client "<< username<< " disconnected!" << std::endl;
                        std::cout << "Total clients: " << clients.size() << std::endl;
                        break;
                    }
                    else
                    {
                        ++iter;
                    }         
                }
            }
        }
        else{ //тогда отправим всем - массовая рассылка.
            std::lock_guard<std::mutex> lock(clients_mutex);
                
                for(const auto& client: clients) {
                    if(client.socket)
                    {
                        std::string response = "[" + username + "]:" + message;
                        boost::asio::write(*client.socket, boost::asio::buffer(response));

                    }
                }
        }
    }
}


// Сохранение сообщения в базу данных
void save_message(const std::string& from_user, const std::string& to_user, const std::string& message, bool is_private, sqlite3* db)
{
   if (!db) return;
   // SQL-запрос с плейсхолдерами ? - местами для подстановки значений
    const char* sql = "INSERT INTO messages (from_user, to_user, message, is_private) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt; // Указатель на подготовленный запрос
    
    // Подготавливаем запрос (варианты ошибок SQLITE_OK (0) - успех SQLITE_ERROR (1) - ошибка в синтаксисе SQLITE_NOMEM (7) - недостаточно памяти)
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    
    // Привязываем значения - подставляет текст вместо ? в запросе.
    sqlite3_bind_text(stmt, 1, from_user.c_str(), -1, SQLITE_STATIC);
    
    if (to_user.empty()) {
        sqlite3_bind_null(stmt, 2);  // Публичное сообщение - нет получателя
    } else {
        sqlite3_bind_text(stmt, 2, to_user.c_str(), -1, SQLITE_STATIC);
    }
    //Подставлям текст сообщения вместо третьего знака ? в SQL-запросе
    sqlite3_bind_text(stmt, 3, message.c_str(), -1, SQLITE_STATIC);
    // подставляем 0 или 1 на место четвертого знака ? в SQL-запросе(преобразуя bool->int).
    sqlite3_bind_int(stmt, 4, is_private ? 1 : 0);
    
    // выполнение запроса
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to insert message: " << sqlite3_errmsg(db) << std::endl;
    }
    
    // Освобождаем память, выделенную под подготовленный запрос (иначе будет утечка памяти).
    sqlite3_finalize(stmt);
}

// Загрузка последних сообщений
void load_messages(const std::string& username, std::shared_ptr<tcp::socket> socket, sqlite3* db, int limit) {

  if(!db) return;
  const char * sql = "SELECT from_user, message, timestamp, is_private FROM messages WHERE to_user IS NULL OR to_user = ? OR from_user = ? ORDER BY timestamp DESC LIMIT ?;";
  sqlite3_stmt * stmt;// Указатель на подготовленный запрос
    
  // Подготавливаем запрос 
  int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
      std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
  
   // Привязываем параметры
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, limit);
    
    // Формируем историю
    std::string history = "\n--- Last messages ---\n";
    int count = 0;
    
    // Читаем результаты
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* from = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* msg = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* time = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        int is_private = sqlite3_column_int(stmt, 3);
        
        // Добавляем маркер для личных сообщений
        if (is_private) {
            history += "[PM] ";
        }
        
        history += std::string(time) + " " + from + ": " + msg + "\n";
        count++;
    }
    if (count > 0) {

        boost::asio::write(*socket, boost::asio::buffer(history));
    } 
    sqlite3_finalize(stmt);
}

