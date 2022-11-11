#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include "Server/server.h"
#include <thread>
#include <memory>
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
