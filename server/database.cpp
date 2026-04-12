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


int database_init(sqlite3* db)
{
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
        
        if (!execute_sql(create_table_sql, db)) {
        std::cerr << "Failed to create table" << std::endl;
        return 1;
    }
    std::cout << "Messages table ready" << std::endl;

    return 0;
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


