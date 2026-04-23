#include "database.h"
#include "server.h"

// Глобальный указатель на базу данных (доступен из других модулей)
sqlite3* db = nullptr;

/**
 * @brief Главная функция: запуск сервера мессенджера
 * @param argc Количество аргументов (должно быть 2)
 * @param argv Аргументы: [0] - имя программы, [1] - порт
 * @return 0 - успех, 1 - ошибка
 * 
 * Пример запуска: ./messenger_server 8080
 */
int main(int argc, char* argv[]) {

    if (argc != 2) {
            std::cerr << "Usage: messenger_server <port>\n";
            return 1;   
    }
    // Преобразование порта из строки в число 
    int port;
    try {
        port = std::stoi(argv[1]);
    } catch (...) {
        std::cerr << "[ERROR] Invalid port number: " << argv[1] << std::endl;
        return 1;
    }

    // Проверка диапазона порта ---
    if (port < 1024 || port > 65535) {
        std::cerr << "[ERROR] Port must be between 1024 and 65535" << std::endl;
        return 1;
    }

    
    // Инициализация БД
    db = database_init();
    if (!db) {
        std::cerr << "Failed to initialize database" << std::endl;
        return 1;
    }
    std::cout << "[OK] Database ready" << std::endl;

     // Запуск сервера
    std::cout << "[START] Launching server on port " << port << "..." << std::endl;
    server_run(port);
    
    // Очистка ресурсов (сюда код дойдёт только после остановки сервера)
    sqlite3_close(db);

    std::cout << "[OK] Server stopped successfully" << std::endl;
    
    return 0;
}
