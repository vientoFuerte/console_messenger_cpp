
#include <iostream>
#include <boost/asio.hpp>
#include <string>
#include <thread>

using boost::asio::ip::tcp;

// Функция для обработки одного клиента
void handle_client(std::shared_ptr<tcp::socket> socket) {
    std::cout << "Client connected!" << std::endl;
    
    while (true) {
        char data[128];
        boost::system::error_code error;
        
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
        
        // Отправляем ответ
        std::string response = "Server echo: " + message;
        boost::asio::write(*socket, boost::asio::buffer(response));
    }
}



int main(int argc, char* argv[]) {

    if (argc != 2) {
            std::cerr << "Usage: messenger_server <port>\n";
            return 1;   
    }
    int port = std::stoi(argv[1]);
    
    try {
        boost::asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
        
        std::cout << "Server started on port 9000" << std::endl;
        
        while (true) {
            // Создаем пустой сокет
            auto socket = std::make_shared<tcp::socket>(io_context);
            
            // accept() - это БЛОКИРУЮЩИЙ вызов
            // Он останавливает программу и ЖДЕТ подключения клиента
            acceptor.accept(*socket);
            
            std::cout << "Client connected! Creating thread." << std::endl;
            std::thread client_thread(handle_client, socket);
            client_thread.detach();
           
            
        }
      
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    
    return 0;
}
