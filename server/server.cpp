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


