#include "database.h"
// Глобальный указатель на БД
sqlite3* global_db = nullptr;

// Хранилище всех подключённых клиентов (сокеты)
std::vector<clientInfo> clients;
std::mutex clients_mutex;

/**
 * @brief Выполняет SQL-запрос, который не возвращает данные
 * @param sql SQL-запрос для выполнения
 * @param db Указатель на открытую базу данных
 * @return true - успех, false - ошибка
 */
bool execute_sql(const std::string& sql, sqlite3* db) {
    char* errMsg = nullptr;
   // sqlite3_exec -функция для выполнения SQL-запросов, которые не возвращают данные (CREATE, INSERT, UPDATE, DELETE).
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);  // Освобождаем память из-под сообщения об ошибке
        return false;
    }
    return true;
}

/**
 * @brief Инициализирует базу данных и создаёт необходимые таблицы
 * @return Указатель на открытую базу данных или nullptr при ошибке
 */
sqlite3* database_init() {
    sqlite3* db = nullptr;
    // Открываем (или создаём) файл базы данных
    int rc = sqlite3_open("messenger.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return nullptr;
    }
    std::cout << "Database opened successfully" << std::endl;
    
    // Создаем таблицу для сообщений
    const char* create_table_sql = 
        "CREATE TABLE IF NOT EXISTS messages ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"          // Уникальный идентификатор
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"  // Временная метка
        "from_user TEXT NOT NULL,"                       // Отправитель
        "to_user TEXT,"                                  // Получатель (NULL для публичных)
        "message TEXT NOT NULL,"                         // Текст сообщения
        "is_private INTEGER DEFAULT 0);";                // Приватное сообщение (1) или публичное (0)
        
    if (!execute_sql(create_table_sql, db)) {
        std::cerr << "Failed to create table" << std::endl;
        sqlite3_close(db);
        return nullptr;
    }
    global_db = db;  // Сохраняем в глобальную переменную
    return db;
}

// Вспомогательная функция для обработки команды выхода
std::string normalize_command(const std::string& input) {
    std::string result = input;
    
    // Удаляем пробелы в начале и конце
    size_t start = result.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) {
        return ""; // Строка только из пробелов
    }
    size_t end = result.find_last_not_of(" \t\n\r");
    result = result.substr(start, end - start + 1);
    
    // Приводим к нижнему регистру
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    
    return result;
}


/**
 * @brief Проверяет, является ли сообщение командой выхода
 * @param msg входное сообщение от клиента.
 */
bool is_quit(const std::string& msg) {
    std::string norm = normalize_command(msg);
    return norm == "q" || norm == "quit";
}

/**
 * @brief Обрабатывает подключение одного клиента в отдельном потоке
 * @param socket Умный указатель на сокет клиента
 */
void handle_client(std::shared_ptr<tcp::socket> socket) {
   
   char data[1024];
   boost::system::error_code error;
   clientInfo client;
   
    // читаем имя клиента
    size_t len = socket->read_some(boost::asio::buffer(data), error);
    
    if (error) {
        std::cout << "Error reading username: " << error.message() << std::endl;
        return;
    }
    //убираем последний символ перевода строки (\n)
    std::string username(data, len -1);
    
    std::cout << "User '" << username << "' connected!" << std::endl;
   
    // Загрузка сообщений для нового клиента
   if (global_db) {
       load_messages(username, socket, global_db, 10);
   }
   //  Добавляем клиента в общий список
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.push_back({socket, username});
        std::cout << "Total clients: " << clients.size() << std::endl;
    }  

    // Основной цикл обработки сообщений от клиента 
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
        // Формируем строку сообщения
        std::string message(data, len);

        if (message.empty()) continue; 

        //std::cout << "Client: " << message << std::endl;
        std::shared_ptr<tcp::socket> target_socket = nullptr;
   
        // Обработка приватного сообщения в формате "@username message" 
        if (message[0] == '@') {    
          size_t space_pos = message.find(' ');
          
          if (space_pos != std::string::npos && space_pos > 1) {
            std::string target_username = message.substr(1, space_pos - 1);
            std::string private_message = message.substr(space_pos + 1);
           
             // Ищем получателя в списке клиентов
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                
                for(const auto& client: clients) {
                  if(client.username == target_username) {
                      target_socket = client.socket;
                      break;
                  }
                }

                // Если получатель найден - отправляем сообщение
                if(target_socket)
                {
                    // Отправляем ответ
                    std::string response = "[" + username + "]:" + private_message;
                    boost::asio::write(*target_socket, boost::asio::buffer(response));

                    // Сохраняем приватное сообщение в БД
                    if (global_db) {
                        save_message(username, target_username, private_message, true, global_db);
                    }
                }

              
            }

          }
        }
        else if (is_quit(message)) //  Обработка отключения клиента
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
            break;  // Выходим из цикла обработки сообщений
        }
        else{ // Массовая рассылка публичного сообщения всем клиентам
            std::lock_guard<std::mutex> lock(clients_mutex);
                
                for(const auto& client: clients) {
                    if(client.socket)
                    {
                        std::string response = "[" + username + "]:" + message;
                        boost::asio::write(*client.socket, boost::asio::buffer(response));

                    }
                }
            // Сохраняем сообщение в БД  (to_user = "")
           if (global_db) {
               save_message(username, "", message, false, global_db);
           }
        }
    }
}


/**
 * @brief Сохраняет сообщение в базу данных
 * @param from_user Имя отправителя
 * @param to_user Имя получателя (пустая строка для публичных сообщений)
 * @param message Текст сообщения
 * @param is_private Признак приватного сообщения
 * @param db Указатель на базу данных
 */
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

/**
 * @brief Загружает последние сообщения для клиента
 * @param username Имя пользователя
 * @param socket Сокет для отправки истории
 * @param db Указатель на базу данных
 * @param limit Количество последних сообщений для загрузки
 */
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
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);  // to_user
    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC);  // from_user
    sqlite3_bind_int(stmt, 3, limit);                                 // LIMIT
    
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

    // Отправляем историю клиенту, если есть сообщения
    if (count > 0) {

        boost::asio::write(*socket, boost::asio::buffer(history));
    } 
    sqlite3_finalize(stmt);
}

