#include "database.h"
#include "server.h"

// Глобальный указатель на базу данных
sqlite3* db = nullptr;


int main(int argc, char* argv[]) {

    if (argc != 2) {
            std::cerr << "Usage: messenger_server <port>\n";
            return 1;   
    }
    int port = std::stoi(argv[1]);
    
    db = database_init();
       
    server_run(port);
    
    // Закрываем базу данных
    sqlite3_close(db);
    
    return 0;
}
