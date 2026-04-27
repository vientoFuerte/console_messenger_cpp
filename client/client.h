#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <memory>  // для std::unique_ptr


using boost::asio::ip::tcp;

class Client {
public:
     /** @brief Конструктор клиента 
     * @param host Адрес сервера (например, localhost)
     * @param port Номер порта сервера
     */
    Client(const std::string& host, int port);

   /** @brief Запуск клиента */
    void Run();           // основной цикл

    /** @brief Деструктор клиента 
     */
    ~Client();

private:
    boost::asio::io_context io_context_;     // Асинхронный контекст
    std::unique_ptr<tcp::socket> socket_;    // TCP сокет
    std::string name_;                       // Имя пользователя

     /** @brief Функция для отправки сообщений пользователя (в отдельном потоке)*/
    void SendMessages();

    /** @brief Фукнция приема сообщений от сервера */
    void ReceiveMessages();
  
};

/**
 * @brief Проверяет, является ли сообщение командой выхода
 * @param msg входное сообщение от клиента.
 */
bool is_quit(const std::string& msg);


