#include "Player.h"

#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#define PLAYER_WELCOME_MSG "Welcome to Cal24 Game!\n"

Player::Player(int clientfd) : m_clientfd(clientfd) {

}

Player::~Player() {
    ::close(m_clientfd);
}

int Player::getClientfd() const {
    return m_clientfd;
}

void Player::setDesk(const std::shared_ptr<Desk>& spDesk) {
    m_spDesk = spDesk;
}

void Player::setReady(bool ready) {
    m_ready = ready;
}

bool Player::getReady() const {
    return m_ready;
}

//是否已经发过牌
bool Player::isCardsSent() const {
    return m_cardsSent;
}

bool Player::sendWelcomeMsg() {
    int sentLength = ::send(m_clientfd, PLAYER_WELCOME_MSG, strlen(PLAYER_WELCOME_MSG), 0);
    return sentLength == static_cast<int>(strlen(PLAYER_WELCOME_MSG));
}

bool Player::sendCards() {
    std::shared_ptr<Desk> spDesk = m_spDesk.lock();
    if (spDesk == nullptr) {
        //玩家处于不在任何桌子上的情况
        return true;
    }

    int cardsStrLen = static_cast<int>(spDesk->sendCards.size());
    int sentLength = ::send(m_clientfd, spDesk->sendCards.c_str(), cardsStrLen, 0);

    if (sentLength == cardsStrLen) {
        m_cardsSent = true;
    }
    else {
        m_cardsSent = false;
    }

    return m_cardsSent;
}

bool Player::sendMsgToClient(const std::string& msg) {
    //std::lock_guard<std::mutex> scopedLock(*m_clientfdToMutex[clientfd]);
    //TODO: 假设消息太长，一次性发不出去，需要处理
    int sentLength = ::send(m_clientfd, msg.c_str(), msg.length(), 0);
    return sentLength == static_cast<int>(msg.length());
}

bool Player::recvData() {
    char clientMsg[32] = { 0 };
    int clientMsgLength = ::recv(m_clientfd, clientMsg, sizeof(clientMsg) / sizeof(clientMsg[0]), 0);
    if (clientMsgLength == 0) {
        return false;
    }

    if (clientMsgLength < 0) {
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            //连接真的出错了
            return false;
        }
        else {
            sleep(1);
            return true;
        }
    }

    m_recvBuf.append(clientMsg, clientMsgLength);

    return true;
}

//ABC
void Player::handleClientMsg(std::string& currentMsg) {
    size_t index = 0;
    int lastPackagePos = 0;

    while (true) {
        if (m_recvBuf[index] == '\n') {
            currentMsg = m_recvBuf.substr(0, index + 1);
            m_recvBuf.erase(0, index + 1);

            //拿到一个完整的包就退出
            return;
        }

        if (index + 1 == m_recvBuf.length())
            return;

        ++index;
    }
}