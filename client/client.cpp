#include <client.h>

class Client {
public:
    Client(const std::string& host, int port);
    void Run();           // основной цикл
    ~Client();

private:
    std::string host_;
    int port_;
    void SendMessages();
    void ReceiveMessages();

};

Client::Client (const std::string& host, int port)
{
  host_=host;
  port_=port;
}


Client::void SendMessages()
{
  
}
Client::void ReceiveMessages()
{
  
}
