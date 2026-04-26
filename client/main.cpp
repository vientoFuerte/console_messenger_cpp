#include "client.h"

/**
 * @brief Главная функция клиентского приложения мессенджера
 * 
 * Запускает клиента, который подключается к серверу по указанному порту
 * 
 * @param argc Количество аргументов командной строки (должно быть равно 2)
 * @param argv Массив аргументов: 
 *            argv[0] - имя программы
 *            argv[1] - номер порта для подключения
 * @return int 0 при успешном завершении, 1 при ошибке
 */

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: messenger_client <port>\n";
        return 1;
    }
    int port = std::stoi(argv[1]);

    std::cout << "CLIENT PROGRAM" << std::endl;

    // Создаём экземпляр клиента с подключением к локальному серверу
    Client client("localhost", port);
    // Запускаем клиента (устанавливаем соединение, обрабатываем сообщения)
    client.Run();

    return 0;
}
