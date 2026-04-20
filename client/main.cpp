#include "client.h"



int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: messenger_client <port>\n";
        return 1;
    }
    int port = std::stoi(argv[1]);

    std::cout << "CLIENT PROGRAM" << std::endl;

    Client client("localhost", port);
    client.Run();


    return 0;
}
