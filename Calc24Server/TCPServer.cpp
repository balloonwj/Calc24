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

#define PLAYER_WELCOME_MSG "Welcome to Cal24 Game!\n"

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

void TCPServer::clientThreadFunc(int clientfd) {
    std::cout << "new client connected, clientfd: " << clientfd << std::endl;

    m_clientfdToRecvBuf[clientfd] = "";
    m_clientfdToMutex[clientfd] = std::move(std::make_shared<std::mutex>());
    m_clientfdToDeskReady[clientfd] = false;

    if (!sendWelcomeMsg(clientfd)) {
        ::close(clientfd);
        return;
    }

    //是否已发牌
    bool initCardsCompleted = false;

    while (true) {
        const auto& deskReady = m_clientfdToDeskReady[clientfd];
        if (deskReady && !initCardsCompleted) {
            if (initCards(clientfd)) {
                std::cout << "initCards successfully, clientfd: " << clientfd << std::endl;

                initCardsCompleted = true;
            }
            else {
                std::cout << "initCards failed, clientfd: " << clientfd << std::endl;

                ::close(clientfd);
                return;
            }
        }

        char clientMsg[32] = { 0 };
        int clientMsgLength = ::recv(clientfd, clientMsg, sizeof(clientMsg) / sizeof(clientMsg[0]), 0);
        if (clientMsgLength == 0) {
            //对端关闭了连接
            ::close(clientfd);
            return;
        }

        if (clientMsgLength < 0) {
            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                //连接真的出错了
                ::close(clientfd);
                return;
            }
            else {
                sleep(1);
                continue;
            }
        }

        std::string& recvBuf = m_clientfdToRecvBuf[clientfd];
        recvBuf.append(clientMsg, clientMsgLength);

        handleClientMsg(clientfd);
    }
}

void TCPServer::newPlayerJoined(int clientfd) {
    Desk* pCurrentDesk = nullptr;

    auto iter = m_deskInfo.rbegin();
    if (iter == m_deskInfo.rend()) {
        Desk newDesk;
        //桌子的id从1开始
        newDesk.id = 1;
        newDesk.clientfd1 = clientfd;
        m_deskInfo.push_back(newDesk);
    }
    else {
        if (iter->clientfd1 == NO_PLAYER_ON_SEAT) {
            iter->clientfd1 = clientfd;
        }
        else if (iter->clientfd2 == NO_PLAYER_ON_SEAT) {
            iter->clientfd2 = clientfd;
        }
        else if (iter->clientfd3 == NO_PLAYER_ON_SEAT) {
            iter->clientfd3 = clientfd;

            //当前来了新玩家之后，这个桌子坐满了
            pCurrentDesk = &(*iter);
        }
        else {
            Desk newDesk;
            newDesk.id = m_deskInfo.size() + 1;
            newDesk.clientfd1 = clientfd;
            m_deskInfo.push_back(newDesk);
        }
    }

    auto spThread = std::make_shared<std::thread>(std::bind(&TCPServer::clientThreadFunc, this, clientfd));
    m_clientfdToThreads[clientfd] = std::move(spThread);

    if (pCurrentDesk != nullptr) {
        std::lock_guard<std::mutex> scopedLock(m_mutexForClientfdToDeskReady);
        //当前来了新玩家之后，这个桌子坐满了，可以发牌了
        m_clientfdToDeskReady[pCurrentDesk->clientfd1] = true;
        m_clientfdToDeskReady[pCurrentDesk->clientfd2] = true;
        m_clientfdToDeskReady[pCurrentDesk->clientfd3] = true;
    }
}

bool TCPServer::sendWelcomeMsg(int clientfd) {
    int sentLength = ::send(clientfd, PLAYER_WELCOME_MSG, strlen(PLAYER_WELCOME_MSG), 0);
    return sentLength == static_cast<int>(strlen(PLAYER_WELCOME_MSG));
}

bool TCPServer::initCards(int clientfd) {
    static constexpr char allCards[] = { 'A', '2', '3', '4', '5', '6', '7', '8', '9', 'X', 'J', 'Q', 'K', 'w', 'W' };
    static constexpr int allCardsCount = sizeof(allCards) / sizeof(allCards[0]);

    int index1 = rand() % allCardsCount;
    int index2 = rand() % allCardsCount;
    int index3 = rand() % allCardsCount;
    int index4 = rand() % allCardsCount;

    char sendCards[24];
    sprintf(sendCards, "Your cards is: %c %c %c %c\n",
        allCards[index1],
        allCards[index2],
        allCards[index3],
        allCards[index4]);
    int sentLength = ::send(clientfd, sendCards, strlen(sendCards), 0);

    return sentLength == static_cast<int>(strlen(sendCards));
}

//ABC
void TCPServer::handleClientMsg(int clientfd) {
    std::string currentMsg;
    size_t index = 0;
    int lastPackagePos = 0;
    std::string& recvBuf = m_clientfdToRecvBuf[clientfd];
    while (true) {
        if (recvBuf[index] == '\n') {
            currentMsg = recvBuf.substr(0, index + 1);
            recvBuf.erase(0, index + 1);

            std::cout << "Client[" << clientfd << "] Says: " << currentMsg << std::endl;

            //转发当前客户端消息给其他玩家
            sendMsgToOtherClients(clientfd, currentMsg);

            if (recvBuf.empty())
                return;

            index = 0;
            continue;
        }

        if (index + 1 == recvBuf.length())
            return;

        ++index;
    }
}

void TCPServer::sendMsgToOtherClients(int msgOwnerClientfd, const std::string& msg) {
    int otherClientfd;
    std::string msgWithOwnerInfo;

    std::lock_guard<std::mutex> scopedLock(m_mutexForClientfdToThreads);
    for (const auto& client : m_clientfdToThreads) {
        otherClientfd = client.first;

        if (otherClientfd == msgOwnerClientfd) {
            continue;
        }

        msgWithOwnerInfo = "Client[" + std::to_string(msgOwnerClientfd) + "] Says: " + msg;
        if (!sendMsgToClient(otherClientfd, msgWithOwnerInfo))
            std::cout << "sendMsgToOtherClients failed, clientfd: " << otherClientfd << std::endl;
    }
}

bool TCPServer::sendMsgToClient(int clientfd, const std::string& msg) {
    std::lock_guard<std::mutex> scopedLock(*m_clientfdToMutex[clientfd]);
    //TODO: 假设消息太长，一次性发不出去，需要处理
    int sentLength = ::send(clientfd, msg.c_str(), msg.length(), 0);
    return sentLength == static_cast<int>(msg.length());
}