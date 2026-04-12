#include "server.h"
#include <thread>

using boost::asio::ip::tcp;


void server_run(int port)
{
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
