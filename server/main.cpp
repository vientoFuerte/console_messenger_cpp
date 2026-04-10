
#include <iostream>
#include <boost/asio.hpp>
#include <string>
#include <thread>

using boost::asio::ip::tcp;

struct clientInfo{
   std::shared_ptr<tcp::socket> socket;
   std::string username;
};

// Хранилище всех подключённых клиентов (сокеты)
std::vector<clientInfo> clients;
std::mutex clients_mutex;

// Функция для обработки одного клиента
void handle_client(std::shared_ptr<tcp::socket> socket) {
   
   char data[128];
   boost::system::error_code error;
   clientInfo client;
   
    // читаем имя клиента
    size_t len = socket->read_some(boost::asio::buffer(data), error);
    
    if (error) {
        std::cout << "Error reading username: " << error.message() << std::endl;
        return;
    }
    //убираем последний символ перевода строки
    std::string username(data, len -1);
    
    std::cout << "User '" << username << "' connected!" << std::endl;
   
   //  Добавляем клиента в общий список
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.push_back({socket, username});
        std::cout << "Total clients: " << clients.size() << std::endl;
    }   
    while (true) {
        
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

   
        // Формат сообщения: "@username message"
        if (message[0] == '@') {    
          size_t space_pos = message.find(' ');
          
          if (space_pos != std::string::npos && space_pos > 1) {
            std::string target_username = message.substr(1, space_pos - 1);
            std::string private_message = message.substr(space_pos + 1);

            std::shared_ptr<tcp::socket> target_socket = nullptr;
            // Ищем получателя
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                
                for(const auto& client: clients)
                {
                  if(client.username == target_username)
                  {
                     target_socket = client.socket;
                  }
                }
                
                if(target_socket)
                {
                    // Отправляем ответ
                    std::string response = "[" + username + "]:" + private_message;
                    boost::asio::write(*target_socket, boost::asio::buffer(response));
                
                }
              
            }

          }
        }
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
    
    return 0;
}
