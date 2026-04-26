#define BOOST_TEST_MODULE MessengerServerTests   //имя модуля тестирования
#include <boost/test/included/unit_test.hpp>
#include <string>
#include "database.h" 

/**
 * @brief Тест создания и инициализации БД
 */
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
    
    const char* table_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));  // Извлекли имя таблицы из столбца 0 (первого)
    BOOST_CHECK_EQUAL(std::string(table_name), "messages");                                // Имя таблицы "messages" 
    
    sqlite3_finalize(stmt);  // Удаляем подготовленный запрос (освобождаем память)
    sqlite3_close(db);       // Закрываем соединение с базой данных
    remove("messenger.db");  // Удаляем тестовый файл
}

/**
 * @brief Тест проверяет, что сервер правильно распознаёт команды выхода
 */
BOOST_AUTO_TEST_CASE(test_quit_commands) {
    // Тестируем созданную функцию
    BOOST_CHECK(is_quit("q"));      // true
    BOOST_CHECK(is_quit("quit"));   // true
    BOOST_CHECK(is_quit("Q"));      // true
    BOOST_CHECK(is_quit(" QUIT ")); // true
    BOOST_CHECK(!is_quit("qwe"));   // false
    BOOST_CHECK(!is_quit(""));      // false
}


/**
 * @brief Тест цикла обработки сообщения
 */
BOOST_AUTO_TEST_CASE(test_message_processing_cycle) {
    // Получаем сообщение от клиента
    std::string message = "@Nata Hello!";
    
    // Определяем тип сообщения
    bool is_private = (message[0] == '@');
    bool is_quit_cmd = is_quit(message);
    
    // Обрабатываем в зависимости от типа
    if (is_quit_cmd) {
        BOOST_CHECK(true); // Клиент отключается
    } 
    else if (is_private) {
        // Извлекаем получателя
        size_t space = message.find(' ');
        std::string recipient = message.substr(1, space - 1);
        std::string msg = message.substr(space + 1);
        
        BOOST_CHECK_EQUAL(recipient, "Nata");
        BOOST_CHECK_EQUAL(msg, "Hello!");
    }
    else { // Публичное сообщение
        BOOST_CHECK_NE(message[0], '@');      // Не начинается с @
        BOOST_CHECK(!is_quit(message));       // Не команда выхода 
    }
}
