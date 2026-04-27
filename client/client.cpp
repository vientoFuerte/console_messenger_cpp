#include "client.h"

/**
 * @brief Конструктор клиента
 * @param host Адрес сервера (например, "localhost")
 * @param port Номер порта для подключения
 * 
 * Создает и инициализирует TCP-сокет, после чего выполняет подключение к серверу.
 * В случае ошибки подключения выбрасывает исключение.
 */

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

/**
 * @brief Деструктор клиента
 * 
 * Закрывает сокет, если он все еще открыт, освобождая системные ресурсы.
 */

Client::~Client() {
    if (socket_ && socket_->is_open()) {
        socket_->close();
    }
}

/**
 * @brief Главный метод запуска клиента
 * 
 * Выполняет инициализацию клиентской сессии:
 * 1. Запрашивает имя пользователя
 * 2. Отправляет имя на сервер
 * 3. Запускает поток для отправки сообщений
 * 4. В основном потоке принимает входящие сообщения
 */

void Client::Run() {
    // Запрос имени пользователя
    std::cout << "Your name: ";
    std::getline(std::cin, name_);
    boost::asio::write(*socket_, boost::asio::buffer(name_ + "\n"));

    std::cout << "Connected. Commands:\n"
            << "@username message — private message\n"
            << "quit / q — exit\n"
            << "Just typing sends public message.\n";

    // запуск потока для отправки сообщений (сокет нельзя копировать, только ссылка)
    std::thread send_messages_thread(&Client::SendMessages, this);

    // Основной поток для приема сообщений
    ReceiveMessages();

    // Ожидание завершения потока отправки
     send_messages_thread.join();
}

// Вспомогательная функция для обработки команды выхода
std::string normalize_command(const std::string& input) {
    std::string result = input;
    
    // Удаляем пробелы в начале и конце
    size_t start = result.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) {
        return ""; // Строка только из пробелов
    }
    size_t end = result.find_last_not_of(" \t\n\r");
    result = result.substr(start, end - start + 1);
    
    // Приводим к нижнему регистру
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    
    return result;
}


/**
 * @brief Проверяет, является ли сообщение командой выхода
 * @param msg входное сообщение от клиента.
 */
bool is_quit(const std::string& msg) {
    std::string norm = normalize_command(msg);
    return norm == "q" || norm == "quit";
}

/**
 * @brief Функция отправки сообщений
 * 
 * Работает в отдельном потоке, читая ввод пользователя из стандартного ввода.
 * Отправляет каждое сообщение на сервер, добавляя символ новой строки.
 * Завершается при вводе команды "quit" или "q" или достижении конца ввода.
 */
void Client::SendMessages()
{
    std::string message;
    while(std::getline(std::cin, message))
    {
        if(is_quit(message)) {
            std::cout << "Disconnecting..." << std::endl;
            break;
        }
        // Пропускаем пустые сообщения
        if (message.empty()) continue;
      
        boost::asio::write(*socket_, boost::asio::buffer(message + "\n"));

    }
      
    socket_->close();

}

/**
 * @brief Функция приема сообщений (основной поток)
 * 
 * Непрерывно читает данные из сокета и выводит их на экран.
 * Работает до тех пор, пока соединение не будет разорвано или не произойдет ошибка.
 * 
 * @note Метод выполняется в главном потоке и блокирует его до завершения чтения.
 */

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
