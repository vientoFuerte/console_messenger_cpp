#pragma once

#include <string>
#include <memory>
#include <boost/asio.hpp>
#include "database.h"

/**
 * @brief Запускает TCP-сервер для мессенджера
 * @param port Номер порта для прослушивания подключений
 * 
 * Функция создаёт TCP-сервер, который принимает входящие подключения
 * и создаёт отдельный поток для обработки каждого клиента.
 */
void server_run(int port);



