#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#define NO_PLAYER_ON_SEAT -1

struct Desk {
    int id;
    int clientfd1{ NO_PLAYER_ON_SEAT };
    int clientfd2{ NO_PLAYER_ON_SEAT };
    int clientfd3{ NO_PLAYER_ON_SEAT };
};

class TCPServer
{
public:
    TCPServer() = default;
    ~TCPServer() = default;

    TCPServer(const TCPServer& rhs) = delete;
    TCPServer& operator=(const TCPServer& rhs) = delete;

    //TODO: 移动拷贝构造函数
    //TODO: 移动operator =

    bool init(const std::string& ip, uint16_t port);

    void start();

    void clientThreadFunc(int clientfd);


    //新玩家加入
    void newPlayerJoined(int clientfd);

    //发送玩家欢迎消息
    bool sendWelcomeMsg(int clientfd);

    //发牌
    bool initCards(int clientfd);

    //处理客户端消息
    void handleClientMsg(int clientfd);

    //转发消息给其他客户端
    void sendMsgToOtherClients(int msgOwnerClientfd, const std::string& msg);

    bool sendMsgToClient(int clientfd, const std::string& msg);


private:
    int                                                     m_listenfd{ -1 };

    //TODO: 未来考虑是否可以将std::shared_ptr改成unique_ptr
    std::unordered_map<int, std::shared_ptr<std::thread>>   m_clientfdToThreads;
    std::mutex                                              m_mutexForClientfdToThreads;

    std::unordered_map<int, std::string>                    m_clientfdToRecvBuf;

    std::unordered_map<int, std::shared_ptr<std::mutex>>    m_clientfdToMutex;

    std::vector<Desk>                                       m_deskInfo;

    //key=>clientfd, value=>clientfd所对应的玩家是否可以进行发牌了，true表示已经可以发牌，false表示玩家数量不够，不能发牌
    std::unordered_map<int, std::atomic<bool>>              m_clientfdToDeskReady;
    std::mutex                                              m_mutexForClientfdToDeskReady;

};
