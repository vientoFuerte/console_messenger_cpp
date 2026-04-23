#include "server.h"
#include <thread>

using boost::asio::ip::tcp;

/**
 * @brief Запуск основного цикла сервера
 * @param port Порт для прослушивания подключений
 * 
 * Сервер работает в бесконечном цикле:
 * 1. Ожидает подключение нового клиента (блокирующий вызов accept)
 * 2. При подключении создаёт отдельный поток для обработки клиента
 * 3. Поток отсоединяется (detach) для параллельной работы
 * 4. Сервер продолжает ожидать новых подключений
 */
void server_run(int port)
{
    try {
        // Инициализация ASIO контекста
        // io_context отвечает за управление асинхронными операциями ввода-вывода
        boost::asio::io_context io_context;
        // Создание акцептора (приёмника подключений)
        // Акцептор привязывается к указанному порту на всех сетевых интерфейсах (tcp::v4())
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
        
        // Выводим информацию о запуске сервера
        std::cout << "========================================" << std::endl;
        std::cout << "   Messenger Server Started Successfully" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "   Listening on port: " << port << std::endl;
        std::cout << "   Waiting for client connections...   " << std::endl;
        std::cout << "========================================" << std::endl << std::endl;

        // Основной цикл сервера 
        while (true) {
            // Создаём умный указатель на новый сокет для клиента
            // Используем shared_ptr для автоматического управления временем жизни сокета
            auto socket = std::make_shared<tcp::socket>(io_context);
            
            // Блокирующий вызов accept() - ожидает подключения нового клиента
            // Программа останавливается здесь до тех пор, пока не подключится клиент
            acceptor.accept(*socket);
            // Обработка подключённого клиента 
            std::cout << "Client connected! Creating thread." << std::endl;
            
            // Создаём отдельный поток для обработки клиента
            // handle_client - функция из database.cpp, обрабатывающая обмен сообщениями
            std::thread client_thread(handle_client, socket);
            // Отсоединяем поток, чтобы он выполнялся независимо от основного цикла
            // Поток будет автоматически завершён после отключения клиента
            client_thread.detach();
           
            
        }
      
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}


