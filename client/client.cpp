#include "client.h"



Client::Client (const std::string& host, int port)
{
  socket_ = std::make_unique<tcp::socket>(io_context_);  //Сетевой сокет - "труба" для обмена данными.
  try {
        tcp::resolver resolver(io_context_);
        auto endpoints = resolver.resolve(host, std::to_string(port));
        boost::asio::connect(*socket_, endpoints);
    } catch (std::exception& e) {
        std::cerr << "Connection error: " << e.what() << std::endl;
        throw;
    }
}

Client::~Client() {
    if (socket_ && socket_->is_open()) {
        socket_->close();
    }
}

void Client::Run() {
    // Запрос имени пользователя
    std::cout << "Your name: ";
    std::getline(std::cin, name_);
    boost::asio::write(*socket_, boost::asio::buffer(name_ + "\n"));

    std::cout << "Connected. Commands:\n"
            << "  @username message — private message\n"
            << "  /quit — exit\n"
            << "Just typing sends public message.\n";

    // запуск потока для отправки сообщений (сокет нельзя копировать, только ссылка)
    std::thread send_messages_thread(&Client::SendMessages, this);

    // Основной поток для приема сообщений
    ReceiveMessages();

    // Ожидание завершения потока отправки
     send_messages_thread.join();
}

void Client::SendMessages()
{
    std::string message;
    while(std::getline(std::cin, message))
    {
        if(message == "quite") {break;}
        boost::asio::write(*socket_, boost::asio::buffer(message + "\n"));

    }
      
    socket_->close();

}
void Client::ReceiveMessages()
{
    std::array<char, 1024> buffer;
    boost::system::error_code error;

    for (;;)
    {
        size_t len = socket_->read_some(boost::asio::buffer(buffer), error);
        
            if (error) {
            std::cout << "ERROR!" << error << std::endl;
            break;
        }

        // Выводим полученное сообщение
        std::cout << std::string(buffer.data(), len) << std::endl;

    }
  
}
