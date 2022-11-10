#include "./server.h"
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
#include <thread>
#include <string>
#include <exception>
#include <limits>
#include <mutex>
#include <memory>


std::vector<std::string> splitRecvData(const std::string &txt, const char &&ch)
{
    std::vector<std::string> strs;
    size_t pos = txt.find( ch );
    size_t initialPos = 0;

    while( pos != std::string::npos ) {
        strs.push_back( txt.substr( initialPos, pos - initialPos ) );
        initialPos = pos + 1;

        pos = txt.find( ch, initialPos );
    }

    strs.push_back( txt.substr( initialPos, std::min( pos, txt.size() ) - initialPos + 1 - 3) );

    return strs;
}



void Server::WorkWithClient(const int socketDesc, const uint32_t clientAddres)
{
    char msg[150] = "To disconnect type Exit\n";
    send(socketDesc, &msg, sizeof(msg), 0);
    std::cout << "addr clint:" << clientAddres << std::endl;
    while (!this->stopFlag)
    {            
        memset(&msg, 0, sizeof(msg)); 
        recv(socketDesc, &msg, sizeof(msg), 0);
        std::cout << msg << " " << sizeof(msg) << std::endl;
        std::string command(msg);
        std::cout << "Client:" << command << std::endl;
        auto args = splitRecvData(command, ' ');
        
        if (args[0] == "exit")
        {
            break;
        }
        else if (args[0].substr(0,3) == "seq" && args[0][3]>= '1' && args[0][3] <= '3')
        {
            try{
                auto numSeq = args[0][4] - '0';
                auto startValue = std::stoi(args[1]);
                auto step = std::stoi(args[2]);
                auto seq = std::make_pair(startValue, step);
                std::scoped_lock lk (this->mt);
                if (this->storage.find(clientAddres) == this->storage.end())
                {
                    std::vector<std::pair<int,int>> vec(3);                
                    vec[numSeq-1] = seq;
                    this->storage[clientAddres] = vec;
                }
                else
                {
                    this->storage[clientAddres][numSeq-1] = seq;
                }
                strcpy(msg, "Seq are set\n");
                send(socketDesc, &msg, sizeof(msg), 0);
            }
            catch(std::exception &e)
            {
                strcpy(msg, "Invalid seq parametrs\n");
                send(socketDesc, &msg, sizeof(msg), 0);
            }
        }
        else if (args[0] == "export" && args[1] == "seq")
        {
            /* code */
        }
        else
        {            
            strcpy(msg, "Unknown command\n");
            send(socketDesc, &msg, sizeof(msg), 0);
        }
    }
    close(socketDesc);
    std::cout << "Client desc closed" << std::endl;
}

void Server::Start(const int port)
{    
    sockaddr_in serverAddres;
    serverAddres.sin_family = AF_INET;
    serverAddres.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddres.sin_port = htons(port);

    //open stream oriented socket with internet address
    //also keep track of the socket descriptor
    int serverSd = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSd < 0)
    {
        std::cerr << "Error establishing the server socket" << std::endl;
        exit(0);
    }
    //bind the socket to its local address
    int bindStatus = bind(serverSd, (struct sockaddr*) &serverAddres,
        sizeof(serverAddres));
    if(bindStatus < 0)
    {
        std::cerr << "Error binding socket to local address" << std::endl;
        exit(0);
    }
    std::cout << "Waiting for a client to connect..." << std::endl;
    //listen for up to 5 requests at a time
    listen(serverSd, 5);
    //receive a request from client using accept
    //we need a new address to connect with the client
    while(!this->stopFlag)
    {
        sockaddr_in clientSocketAddres;
        socklen_t clinetSocketAddresSize = sizeof(clientSocketAddres);
        //accept, create a new socket descriptor to
        //handle the new connection with client
        int newSd = accept(serverSd, (sockaddr *)&clientSocketAddres, &clinetSocketAddresSize);
        if(newSd < 0)
        {
            std::cerr << "Error accepting request from client!" << std::endl;
            exit(1);
        }
        std::cout << "Connected with client!" << std::endl;
        std::cout << newSd << std::endl;
        std::thread newClient (&Server::WorkWithClient, this, newSd, clientSocketAddres.sin_addr.s_addr);
        newClient.detach();        
    }
    //we need to close the socket descriptors after we're all done   
    
    close(serverSd);
   
    std::cout << "Server closed" << std::endl;
}

void Server::Stop(){
    this->stopFlag = true;
}

