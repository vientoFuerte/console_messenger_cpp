#define BOOST_TEST_MODULE MessengerServerTests   //имя модуля тестирования
#include <boost/test/included/unit_test.hpp>
#include <string>
#include "database.h" 

// Тест БД
BOOST_AUTO_TEST_CASE(test_database_creation) {
    sqlite3* db =  database_init(); 
    
    BOOST_CHECK(db != nullptr);
  
   // Проверяем, что таблица messages существует
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, 
        "SELECT name FROM sqlite_master WHERE type='table' AND name='messages';",
        -1, &stmt, nullptr);
    
    BOOST_CHECK_EQUAL(rc, SQLITE_OK);
    BOOST_CHECK_EQUAL(sqlite3_step(stmt), SQLITE_ROW);
    
    const char* table_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    BOOST_CHECK_EQUAL(std::string(table_name), "messages");
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    remove("messenger.db");  // Удаляем тестовый файл
}
