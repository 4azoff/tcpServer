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
#include <limits.h>
#include <mutex>
#include <memory>

struct Counter
{
    unsigned long long counter;
    int step;
    int firstValue;
    Counter(unsigned long long _counter, int _step, int firstVal) : counter{_counter}, step{_step},
                                                                    firstValue{firstVal} {};
};

std::vector<std::string> splitRecvData(const std::string &txt, const char &&ch)
{
    std::vector<std::string> strs;
    size_t pos = txt.find(ch);
    size_t initialPos = 0;

    while (pos != std::string::npos)
    {
        strs.push_back(txt.substr(initialPos, pos - initialPos));
        initialPos = pos + 1;

        pos = txt.find(ch, initialPos);
    }

    strs.push_back(txt.substr(initialPos, std::min(pos, txt.size()) - initialPos + 1 - 3));

    return strs;
}

void Server::ExportSeq(const int &socketDesc, const uint32_t &clientAddres)
{
    if (this->storage.find(clientAddres) != this->storage.end())
    {
        std::vector<Counter> sequences;
        for (const auto &seq : this->storage[clientAddres])
        {
            if (seq.second.first != 0 && seq.second.first != 0)
            {
                sequences.emplace_back(seq.second.first, seq.second.second, seq.second.first);
            }
        }
        while (!this->stopFlag)
        {
            std::string tmp;
            for (auto &elem : sequences)
            {
                tmp += std::to_string(elem.counter) + " ";
                if (elem.counter + elem.step > ULLONG_MAX)
                {
                    elem.counter = elem.firstValue;
                }
                else
                {
                    elem.counter += elem.step;
                }
            }
            tmp += "\n";
            send(socketDesc, tmp.c_str(), sizeof(tmp), 0);
            //std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        }
    }
    else
    {
        std::string errorMessage = "You have not set any sequence\n";
        send(socketDesc, errorMessage.c_str(), sizeof(errorMessage), 0);
    }
}

void Server::WorkWithClient(const int socketDesc, const uint32_t clientAddres)
{
    char msg[150] = "To disconnect type "
                    "exit"
                    "\n";
    send(socketDesc, &msg, sizeof(msg), 0);
    std::cout << "addr clint:" << clientAddres << std::endl;
    while (!this->stopFlag)
    {
        memset(&msg, 0, sizeof(msg));
        recv(socketDesc, &msg, sizeof(msg), 0);
        std::string command(msg);
        auto args = splitRecvData(command, ' ');

        if (args[0] == "exit")
        {
            break;
        }
        else if (args[0].substr(0, 3) == "seq" && args[0][3] >= '1' && args[0][3] <= '3')
        {
            try
            {
                auto numSeq = args[0][3] - '0';
                auto startValue = std::stoi(args[1]);
                auto step = std::stoi(args[2]);
                auto seq = std::make_pair(startValue, step);

                std::scoped_lock lk(this->mt);
                this->storage[clientAddres][numSeq] = seq;

                strcpy(msg, "Seq are set\n");
                send(socketDesc, &msg, sizeof(msg), 0);
            }
            catch (std::exception &e)
            {
                strcpy(msg, "Invalid seq parametrs\n");
                send(socketDesc, &msg, sizeof(msg), 0);
            }
        }
        else if (args[0] == "export" && args[1] == "seq")
        {
            ExportSeq(socketDesc, clientAddres);
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

    int serverSd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSd < 0)
    {
        std::cerr << "Error establishing the server socket" << std::endl;
        exit(0);
    }

    int bindStatus = bind(serverSd, (struct sockaddr *)&serverAddres,
                          sizeof(serverAddres));
    if (bindStatus < 0)
    {
        std::cerr << "Error binding socket to local address" << std::endl;
        exit(0);
    }
    std::cout << "Waiting for a client to connect..." << std::endl;

    listen(serverSd, 10);

    while (!this->stopFlag)
    {
        sockaddr_in clientSocketAddres;
        socklen_t clinetSocketAddresSize = sizeof(clientSocketAddres);

        int newSd = accept(serverSd, (sockaddr *)&clientSocketAddres, &clinetSocketAddresSize);
        if (newSd < 0)
        {
            std::cerr << "Error accepting request from client!" << std::endl;
            exit(1);
        }
        std::cout << "Connected with client" << std::endl;

        std::thread newClient(&Server::WorkWithClient, this, newSd, clientSocketAddres.sin_addr.s_addr);
        newClient.detach();
    }    

    close(serverSd);

    std::cout << "Server closed" << std::endl;
}

void Server::Stop()
{
    this->stopFlag = true;
}
