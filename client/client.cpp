#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <thread>

using boost::asio::ip::tcp;

class Client {
public:
    Client(const std::string& host, int port);
    void Run();           // основной цикл
    ~Client();

private:
    boost::asio::io_context io_context_;
    std::unique_ptr<tcp::socket> socket_;
    std::string name_;

    
    void SendMessages(tcp::socket& socket);  // Функция для отправки сообщений в отдельном потоке
    void ReceiveMessages();
  
};





