#include "TCPServer.h"

#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include <iostream>
#include <functional>

#include "Player.h"

bool TCPServer::init(const std::string& ip, uint16_t port) {
    srand(time(nullptr));

    //1.创建一个侦听socket
    m_listenfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenfd == -1)
        return false;

    int optval = 1;
    //TODO: 判断一下这两个函数是否调用成功
    ::setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval));
    ::setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof optval));

    //2.初始化服务器地址
    struct sockaddr_in bindaddr;
    bindaddr.sin_family = AF_INET;
    bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bindaddr.sin_port = htons(port);
    if (::bind(m_listenfd, (struct sockaddr*)&bindaddr, sizeof(bindaddr)) == -1) {
        std::cout << "bind listen socket error." << std::endl;
        return false;
    }

    //3.启动侦听
    if (::listen(m_listenfd, SOMAXCONN) == -1) {
        std::cout << "listen error." << std::endl;
        return false;
    }

    return true;
}

void TCPServer::start() {
    std::cout << "start TCPServer successfully" << std::endl;

    while (true) {
        struct sockaddr_in clientaddr;
        socklen_t clientaddrlen = sizeof(clientaddr);
        //4. 接受客户端连接
        int clientfd = ::accept4(m_listenfd, (struct sockaddr*)&clientaddr, &clientaddrlen, SOCK_NONBLOCK);
        if (clientfd == -1) {
            std::cout << "accept error." << std::endl;
            return;
        }

        newPlayerJoined(clientfd);
    }
}

void TCPServer::clientThreadFunc(std::shared_ptr<Player> spPlayer, std::shared_ptr<Desk> spOwnerDesk) {
    int clientfd = spPlayer->getClientfd();
    std::cout << "new client connected, clientfd: " << clientfd << std::endl;

    if (spPlayer->sendWelcomeMsg()) {
        bool exit = false;
        while (!exit) {
            if (spPlayer->getReady() && !spPlayer->isCardsSent()) {
                if (spPlayer->sendCards()) {
                    std::cout << "sendCards to clientfd[" << clientfd << "] successfully" << std::endl;
                }
                else {
                    std::cout << "sendCards to clientfd[" << clientfd << "] failed" << std::endl;
                    exit = true;
                    continue;
                }
            }// end if

            if (!spPlayer->recvData()) {
                std::cout << "recv failed, clientfd[" << clientfd << "]" << std::endl;
                exit = true;
                continue;
            }// end if

            while (true) {
                std::string aMsg;
                spPlayer->handleClientMsg(aMsg);
                if (!aMsg.empty()) {
                    std::cout << "client[" << clientfd << "] Says: " << aMsg << std::endl;

                    sendMsgToOtherClients(clientfd, aMsg);
                }
                else {
                    break;
                }
            }// end inner-while
        }// end outer-while
    }

    //清理clientfd
    std::lock_guard<std::mutex> scopedLock(m_mutexForPlayers);
    m_players.erase(clientfd);
    m_clientfdToThreads.erase(clientfd);
}

void TCPServer::newPlayerJoined(int clientfd) {
    auto spCurrentPlayer = std::make_shared<Player>(clientfd);
    std::shared_ptr<Desk> spCurrentFullDesk;

    auto iter = m_deskInfo.rbegin();
    if (iter == m_deskInfo.rend()) {
        //第一个玩家加入
        //桌子的id从1开始
        auto spDesk = std::make_shared<Desk>();
        //TODO: 1改成常量
        spDesk->id = 1;
        spDesk->spPlayer1 = spCurrentPlayer;

        spCurrentPlayer->setDesk(spDesk);

        m_deskInfo.push_back(std::move(spDesk));
    }
    else {
        if ((*iter)->spPlayer1.expired()) {
            (*iter)->spPlayer1 = spCurrentPlayer;
        }
        else if ((*iter)->spPlayer2.expired()) {
            (*iter)->spPlayer2 = spCurrentPlayer;
        }
        else if ((*iter)->spPlayer3.expired()) {
            (*iter)->spPlayer3 = spCurrentPlayer;

            //当前来了新玩家之后，这个桌子坐满了
            spCurrentFullDesk = *iter;
        }
        else {
            auto spNewDesk = std::make_shared<Desk>();
            spNewDesk->id = m_deskInfo.size() + 1;
            spNewDesk->spPlayer1 = spCurrentPlayer;
            spCurrentPlayer->setDesk(spNewDesk);

            m_deskInfo.push_back(std::move(spNewDesk));
        }
    }

    if (spCurrentFullDesk != nullptr) {
        //当前来了新玩家之后，这个桌子坐满了，可以发牌了
        std::shared_ptr<Player> sp1 = spCurrentFullDesk->spPlayer1.lock();
        if (sp1 != nullptr) {
            sp1->setReady(true);
            sp1->setDesk(spCurrentFullDesk);
        }

        std::shared_ptr<Player> sp2 = spCurrentFullDesk->spPlayer2.lock();
        if (sp2 != nullptr) {
            sp2->setReady(true);
            sp2->setDesk(spCurrentFullDesk);
        }

        std::shared_ptr<Player> sp3 = spCurrentFullDesk->spPlayer3.lock();
        if (sp3 != nullptr) {
            sp3->setReady(true);
            sp3->setDesk(spCurrentFullDesk);
        }


        generateCards(spCurrentFullDesk);
    }

    auto spThread = std::make_shared<std::thread>(std::bind(&TCPServer::clientThreadFunc, this, spCurrentPlayer, spCurrentFullDesk));
    spThread->detach();
    m_clientfdToThreads[clientfd] = std::move(spThread);

    m_players[clientfd] = std::move(spCurrentPlayer);
}

void TCPServer::generateCards(const std::shared_ptr<Desk>& spDesk) {
    static constexpr char allCards[] = { 'A', '2', '3', '4', '5', '6', '7', '8', '9', 'X', 'J', 'Q', 'K', 'w', 'W' };
    static constexpr int allCardsCount = sizeof(allCards) / sizeof(allCards[0]);

    int index1 = rand() % allCardsCount;
    int index2 = rand() % allCardsCount;
    int index3 = rand() % allCardsCount;
    int index4 = rand() % allCardsCount;

    char newCards[24];
    sprintf(newCards, "Your cards is: %c %c %c %c\n",
        allCards[index1],
        allCards[index2],
        allCards[index3],
        allCards[index4]);

    spDesk->sendCards.append(newCards, strlen(newCards));
}

void TCPServer::sendMsgToOtherClients(int msgOwnerClientfd, const std::string& msg) {
    int otherClientfd;
    std::string msgWithOwnerInfo;

    std::lock_guard<std::mutex> scopedLock(m_mutexForPlayers);
    for (const auto& client : m_players) {
        otherClientfd = client.first;

        if (otherClientfd == msgOwnerClientfd) {
            continue;
        }

        msgWithOwnerInfo = "Client[" + std::to_string(msgOwnerClientfd) + "] Says: " + msg;
        if (!client.second->sendMsgToClient(msgWithOwnerInfo))
            std::cout << "sendMsgToOtherClients failed, clientfd: " << otherClientfd << std::endl;
    }
}