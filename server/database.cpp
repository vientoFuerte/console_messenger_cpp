#include "database.h"



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




