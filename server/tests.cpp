#define BOOST_TEST_MODULE MessengerServerTests   //имя модуля тестирования
#include <boost/test/included/unit_test.hpp>
#include <string>
#include "database.h" 

// Тест создания БД
BOOST_AUTO_TEST_CASE(test_database_creation) {
    sqlite3* db =  database_init(); 
    
    BOOST_CHECK(db != nullptr);
  
   // Проверяем, что таблица messages существует
   // Для этого выполняем SQL-запрос к системной таблице sqlite_master
   // sqlite_master - специальная таблица, где SQLite хранит метаданные о всех объектах БД
    
    sqlite3_stmt* stmt; // Указатель на запрос (prepared statement)
    // Подготавка SQL-запроса
    int rc = sqlite3_prepare_v2(db, 
        "SELECT name FROM sqlite_master WHERE type='table' AND name='messages';",
        -1, &stmt, nullptr);
    
    BOOST_CHECK_EQUAL(rc, SQLITE_OK);                  // Запрос скомпилировался без ошибок
    BOOST_CHECK_EQUAL(sqlite3_step(stmt), SQLITE_ROW); // Запрос вернул строку
    
    const char* table_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));  // Извлекли имя таблицы из столбца
    BOOST_CHECK_EQUAL(std::string(table_name), "messages");                                // Имя таблицы "messages" 
    
    sqlite3_finalize(stmt);  // Удаляем подготовленный запрос (освобождаем память)
    sqlite3_close(db);       // Закрываем соединение с базой данных
    remove("messenger.db");  // Удаляем тестовый файл
}
