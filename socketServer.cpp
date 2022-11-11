#include <iostream>
#include <string>
#include "Server/server.h"
#include <exception>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Specify a port" << std::endl;
        exit(0);
    }
    int port;
    try
    {
        port = std::stoi(argv[1]);
    }
    catch (std::exception e)
    {
        std::cerr << "Incorrect port" << std::endl;
        exit(0);
    }
    
    auto server = std::make_shared<Server>();

    std::thread serverThread(&Server::Start, server, port);

    std::cout << "Press any key to stop" << std::endl;
    std::cin.get();
    server->Stop();

    serverThread.join();

    return 0;
}
