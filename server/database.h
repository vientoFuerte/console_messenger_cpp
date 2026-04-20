#pragma once

#include <memory>
#include <iostream>
#include <sqlite3.h>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

struct clientInfo{
   std::shared_ptr<tcp::socket> socket;
   std::string username;
};



// Хранилище всех подключённых клиентов (сокеты)
extern std::vector<clientInfo> clients;
extern std::mutex clients_mutex;


int database_init(sqlite3* db);

// Функция для обработки одного клиента
void handle_client(std::shared_ptr<tcp::socket> socket);

// Сохранение сообщения в базу данных
void save_message(const std::string& from_user, const std::string& to_user, const std::string& message, bool is_private, sqlite3* db);

// Загрузка последних сообщений
void load_messages(const std::string& username, std::shared_ptr<tcp::socket> socket, sqlite3* db, int limit = 5);
