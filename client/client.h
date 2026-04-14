#pragma once

#include <boost/asio.hpp>
#include <string>

class Client {
public:
    Client(const std::string& host, int port);
    void Run();           // основной цикл
    ~Client();

private:
    void SendMessages();
    void ReceiveMessages();
  
};
