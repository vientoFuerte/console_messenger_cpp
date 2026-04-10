#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <thread>

using boost::asio::ip::tcp;

const int HASH_SIZE = 4;


std::string GetMessageHash(std::array<char, 128>& bytes,size_t len)
{
    std::string result;

    int start_index = len - HASH_SIZE;
    int end_index = len; //bytes.size() - 1 - HASH_SIZE;

    for (int i = start_index; i < end_index; ++i) {
        result += bytes.data()[i];
    }
        
    return result;
}

std::string GetFullMessage(std::array<char, 128>& bytes, size_t len)
{
    std::string result;

    int start_index = 0;
    int end_index = len - 1; //bytes.size() - 1 - HASH_SIZE;

    for (int i = start_index; i < end_index; ++i) {
        result += bytes.data()[i];
    }

    return result;
}

std::string GetMessageBody(std::array<char, 128>& bytes, size_t len)
{
    std::string result;

    int start_index = 0;
    int end_index = len - HASH_SIZE; //bytes.size() - 1 - HASH_SIZE;

    for (int i = start_index; i < end_index; ++i) {
        result += bytes.data()[i];
    }

    return result;
}

//Полный дамп байтов

void DumpBuffer(std::array<char, 128>& buffer)
{
    std::cout << "============BUFFER DUMP====================" << std::endl;
    for (auto x : buffer) {
        std::cout << x;
    }
        

    std::cout << "============================================" << std::endl;
}

// Функция для отправки сообщений в отдельном потоке
void send_messages(tcp::socket& socket) {
    std::string message;
    while (true) {
        std::getline(std::cin, message);
        
        if (message == "quit") {
            break;
        }
        
        if (!message.empty()) {
            boost::system::error_code error;
            boost::asio::write(socket, boost::asio::buffer(message), error);
            
            if (error) {
                std::cout << "\n[Send error: " << error.message() << "]" << std::endl;
                break;
            }
        }
    }
}


int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: messenger_client <port>\n";
        return 1;
    }
    int port = std::stoi(argv[1]);
    std::string cmd;
    std::cout << "CLIENT PROGRAM" << std::endl;

    try
    {
        boost::asio::io_context io_context;  //объект, который управляет всеми асинхронными операциями ввода-вывода.
        tcp::resolver resolver(io_context);  //для преобразования имен хостов в IP-адреса.
        
        tcp::socket socket(io_context);      //Сетевой сокет - "труба" для обмена данными.

        std::array<char, 128> buffer;
        boost::system::error_code error;

        tcp::resolver::results_type endpoints =
            resolver.resolve("localhost", std::to_string(port));
        // Подключаемся один раз до цикла.
        boost::asio::connect(socket, endpoints);
        
        // Отправка своего имени
        std::cout << "Your name: ";
        std::string my_name;
        std::getline(std::cin, my_name);
        
        boost::asio::write(socket, boost::asio::buffer(my_name + "\n"))
        std::cout << "Connected. Commands:\n"
                  << "  @username message — private message\n"
                  << "  /quit — exit\n"
                  << "Just typing sends public message.\n";
        
        // запуск потока для отправки сообщений (сокет нельзя копировать, только ссылка)
        std::thread send_messages_thread(send_messages, std::ref(socket));
        
  
        // В главном потоке только читаем сообщения
        for (;;)
        {
            size_t len = socket.read_some(boost::asio::buffer(buffer), error);
            
             if (error) {
                std::cout << "ERROR!" << error << std::endl;
                 break;
            }

            // Выводим полученное сообщение
            std::cout << "\n[Server]: " << std::string(buffer.data(), len) << std::endl;
  
        }
        
        send_messages_thread.join();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
