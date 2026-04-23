#pragma once

#include <memory>
#include <iostream>
#include <sqlite3.h>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

/**
 * @brief Структура для хранения информации о подключённом клиенте
 */
struct clientInfo {
    std::shared_ptr<tcp::socket> socket;  // Умный указатель на сокет клиента
    std::string username;                  // Имя пользователя
};

// Глобальные переменные для управления клиентами
extern std::vector<clientInfo> clients;      // Вектор всех подключённых клиентов
extern std::mutex clients_mutex;             // Мьютекс для потокобезопасного доступа к clients

// Функции для работы с базой данных
sqlite3* database_init();                                           // Инициализация БД
void handle_client(std::shared_ptr<tcp::socket> socket);           // Обработка клиента
void save_message(const std::string& from_user, const std::string& to_user, 
                  const std::string& message, bool is_private, sqlite3* db);  // Сохранение сообщения
void load_messages(const std::string& username, std::shared_ptr<tcp::socket> socket, 
                   sqlite3* db, int limit = 5);                    // Загрузка истории сообщений
