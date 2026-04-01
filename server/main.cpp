
#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main(int argc, char* argv[]) {

    if (argc != 2) {
            std::cerr << "Usage: join_server <port>\n";
            return 1;
    
    }
    
    try {
        boost::asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 9000));
        
        std::cout << "Server started on port 9000" << std::endl;
        
        while (true) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);
            
            std::cout << "Client connected!" << std::endl;
            
            // Читаем сообщение
            char data[128];
            boost::system::error_code error;
            size_t len = socket.read_some(boost::asio::buffer(data), error);
            
            if (!error) {
                std::string message(data, len);
                std::cout << "Received: " << message << std::endl;
                
                // Отправляем ответ
                std::string response = "Echo: " + message;
                boost::asio::write(socket, boost::asio::buffer(response));
            }
            
            socket.close();
        }
        
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    
    return 0;
}
