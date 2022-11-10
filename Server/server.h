#include <map>
#include <vector>
#include <mutex>
#include <atomic>
#include <string>
#include <unordered_map>

class Server
{
private: 

    std::unordered_map<uint32_t, std::map<int, std::pair<int,int>>> storage;
    mutable std::mutex mt;
    std::atomic_bool stopFlag {false};

public:

    void WorkWithClient(const int socketDesc, const uint32_t clientAddres);
    void ExportSeq(const int& socketDesc, const uint32_t& clientAddres);
    void Start(const int port);
    void Stop();

};
