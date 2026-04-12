#pragma once

#include <string>
#include <memory>
#include <boost/asio.hpp>
#include "database.h"


void server_run(int port);

// Функция для выполнения SQL-запросов
bool execute_sql(const std::string& sql);

// Сохранение сообщения в базу данных
void save_message(const std::string& from_user, const std::string& to_user, const std::string& message, bool is_private, sqlite3* dbs);

// Загрузка последних сообщений
void load_messages(const std::string& username, std::shared_ptr<tcp::socket> socket, sqlite3* db, int limit = 5);

