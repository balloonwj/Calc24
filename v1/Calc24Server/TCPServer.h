#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

class Player;
class Desk;

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

    void clientThreadFunc(std::shared_ptr<Player> spPlayer, std::shared_ptr<Desk> spDesk);


    //新玩家加入
    void newPlayerJoined(int clientfd);

    //生成待发的牌
    void generateCards(const std::shared_ptr<Desk>& spDesk);

    //转发消息给其他客户端
    void sendMsgToOtherClients(int msgOwnerClientfd, const std::string& msg);


private:
    int                                                     m_listenfd{ -1 };

    //TODO: 未来考虑是否可以将std::shared_ptr改成unique_ptr
    std::unordered_map<int, std::shared_ptr<std::thread>>   m_clientfdToThreads;
    //std::mutex                                              m_mutexForClientfdToThreads;

    //key=>clientfd, value=>std::shared_ptr<Player>
    std::unordered_map<int, std::shared_ptr<Player>>        m_players;
    std::mutex                                              m_mutexForPlayers;


    std::vector<std::shared_ptr<Desk>>                      m_deskInfo;
};
