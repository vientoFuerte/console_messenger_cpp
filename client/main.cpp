// boost_visualcpp_template.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>

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

int main()
{
    std::string cmd;
    std::cout << "CLIENT PROGRAM" << std::endl;

    try
    {
        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);

        tcp::socket socket(io_context);
        

        std::array<char, 128> buffer;
        boost::system::error_code error;

        tcp::resolver::results_type endpoints =
            resolver.resolve("localhost", "9000");


        for (;;)
        {
            

            std::cin >> cmd;

            boost::asio::connect(socket, endpoints);
            size_t len = socket.read_some(boost::asio::buffer(buffer), error);
            


            if (!error) {
                               
                std::string msg_hash = GetMessageHash(buffer, len);
                std::string full_message = GetFullMessage(buffer, len);
                std::string msg_body = GetMessageBody(buffer, len);
                

                std::cout << "FULL MESSAGE:" << full_message << std::endl;
                std::cout << "MESSAGE BODY:" << msg_body << std::endl;
                std::cout << "MESSAGE HASH:" << msg_hash << std::endl;
                DumpBuffer(buffer);
                //std::cout.write(buffer.data(),len); // Вывод данных буфера
                
            }                 
            else if (error) {
                std::cout << "ERROR!" << error << std::endl;
            }
                

            

            
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}

