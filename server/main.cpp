#include <iostream>
#include <boost/asio.hpp>
#include <string>
#include <thread>
#include <sqlite3.h>

using boost::asio::ip::tcp;

struct clientInfo{
   std::shared_ptr<tcp::socket> socket;
   std::string username;
};

// Глобальный указатель на базу данных
sqlite3* db = nullptr;

// Хранилище всех подключённых клиентов (сокеты)
std::vector<clientInfo> clients;
std::mutex clients_mutex;

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
        std::cout << "Client: " << message << std::endl;

   
        // Формат сообщения: "@username message"
        if (message[0] == '@') {    
          size_t space_pos = message.find(' ');
          
          if (space_pos != std::string::npos && space_pos > 1) {
            std::string target_username = message.substr(1, space_pos - 1);
            std::string private_message = message.substr(space_pos + 1);

            std::shared_ptr<tcp::socket> target_socket = nullptr;
            // Ищем получателя
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                
                for(const auto& client: clients)
                {
                  if(client.username == target_username)
                  {
                     target_socket = client.socket;
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
    }
}

// Функция для выполнения SQL-запросов
bool execute_sql(const std::string& sql) {
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

// Сохранение сообщения в базу данных
void save_message(const std::string& from_user, const std::string& to_user, const std::string& message, bool is_private)
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
void load_messages(const std::string& username, std::shared_ptr<tcp::socket> socket, int limit = 5) {

}




int main(int argc, char* argv[]) {

    if (argc != 2) {
            std::cerr << "Usage: messenger_server <port>\n";
            return 1;   
    }
    int port = std::stoi(argv[1]);
    
    // Открываем базу данных
    int rc = sqlite3_open("messenger.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
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
        
        if (!execute_sql(create_table_sql)) {
        std::cerr << "Failed to create table" << std::endl;
        return 1;
    }
    std::cout << "Messages table ready" << std::endl;
    
        
    try {
        boost::asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
        
        std::cout << "Server started on port "<<port << std::endl;
        
        while (true) {
            // Создаем пустой сокет
            auto socket = std::make_shared<tcp::socket>(io_context);
            
            // accept() - это блокирующий вызов
            // он останавливает программу и ждет подключения клиента
            acceptor.accept(*socket);
            
            std::cout << "Client connected! Creating thread." << std::endl;
            std::thread client_thread(handle_client, socket);
            client_thread.detach();
           
            
        }
      
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    
    // Закрываем базу данных
    sqlite3_close(db);
    
    return 0;
}
