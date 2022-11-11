#include <map>
#include <vector>
#include <shared_mutex>
#include <atomic>
#include <string>
#include <unordered_map>
#include <thread>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <exception>
#include <limits.h>


class Server
{
private: 

    std::unordered_map<uint32_t, std::map<int, std::pair<int,int>>> storage;
    mutable std::shared_mutex mt;
    std::atomic_bool stopFlag {false};

public:

    void WorkWithClient(const int socketDesc, const uint32_t clientAddres);
    void ExportSeq(const int& socketDesc, const uint32_t& clientAddres);
    void Start(const int port);
    void Stop();

};
