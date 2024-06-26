﻿#include "Calc24Session.h"

#include <functional>
#include <iostream>
#include <sstream>
#include <string.h>

#include "Calc24Server.h"

#define MAX_MSG_LENGTH 64

Calc24Session::Calc24Session(Calc24Server* pServer, std::shared_ptr<TCPConnection>&& spConn) :
    m_pServer(pServer),
    m_spConn(std::move(spConn)) {

    m_id = Calc24Session::generateID();

    m_spConn->setReadCallback(std::bind(&Calc24Session::onRead, this, std::placeholders::_1));
    m_spConn->setWriteCallback(std::bind(&Calc24Session::onWrite, this));
    m_spConn->setCloseCallback(std::bind(&Calc24Session::onClose, this));

    std::cout << "Calc24Session::ctor, id: " << m_id << std::endl;
}

Calc24Session::~Calc24Session() {
    std::cout << "Calc24Session::ctor" << m_id << std::endl;
}

void Calc24Session::onRead(ByteBuffer& recvBuf) {
    while (true) {
        DecodePackageResult result = decodePackage(recvBuf);
        if (result == DecodePackageResult::DecodePackageWantMoreData)
            return;

        if (result == DecodePackageResult::DecodePackageFailed) {
            forceClose();
            return;
        }

        //继续下一个包的解析
    }
}

void Calc24Session::onWrite() {

}

void Calc24Session::onClose() {
    //Calc24Session对象做自己一些要清理的逻辑

    //std::shared_ptr<Calc24Session> sp1 = shared_from_this();
    //TODO: 为什么传shared_from_this()不行？
    //m_pServer->onDisconnected(shared_from_this());

    m_pServer->onDisconnected(m_id);
}

void Calc24Session::sendMsg(const std::string& msg) {
    m_spConn->send(msg);
}

void Calc24Session::sendWelcomeMsg() {
    const std::string& welcomeMsg = "Welcome to Calc24 Game.\n";
    m_spConn->send(welcomeMsg);
}

void Calc24Session::notifyUserToBeReady() {
    m_spConn->getEventLoop()->addTimer(5000, true, 0, [this](int64_t timerID) -> void {
        if (m_status == SESSION_STATUS_IDLE)
            m_spConn->send("please handup...\n");
        else {
            m_spConn->getEventLoop()->removeTimer(timerID);
        }
        });
}

void Calc24Session::sendCards(const std::string& cards, int64_t deskID) {
    m_spConn->send(cards);

    m_status = SESSION_STATUS_IN_GAME;

    m_deskID = deskID;
}

void Calc24Session::forceClose() {
    m_spConn->onClose();
}

void Calc24Session::resetToIdle() {
    m_deskID = 0;
    m_status = SESSION_STATUS_IDLE;

    notifyUserToBeReady();
}

int Calc24Session::generateID() {
    static std::atomic<int32_t> m_baseID{ 0 };

    int32_t resultID = ++m_baseID;
    return resultID;
}

DecodePackageResult Calc24Session::decodePackage(ByteBuffer& recvBuf) {
    size_t positionLF = recvBuf.findLF();
    if (positionLF == std::string::npos) {
        if (recvBuf.remaining() > MAX_MSG_LENGTH)
            return DecodePackageResult::DecodePackageFailed;

        return DecodePackageResult::DecodePackageWantMoreData;
    }

    std::string currentPackage;
    recvBuf.retrieve(currentPackage, positionLF + 1);

    processPackage(currentPackage);

    return DecodePackageResult::DecodePackageSuccess;
}

bool Calc24Session::processPackage(const std::string& package) {
    if (package.empty())
        return true;

    if (package[0] != '!') {
        //聊天消息
        return processChatMsg(package);
    } else {
        //特殊指令
        return processCmd(package);
    }
}

bool Calc24Session::processChatMsg(const std::string& package) {
    std::cout << "fd " << m_spConn->fd() << ", client[" << m_id << "] says: " << package << std::endl;

    std::ostringstream msgWithPrefix;
    msgWithPrefix << "client[";
    msgWithPrefix << m_id;
    msgWithPrefix << "] Says: ";
    msgWithPrefix << package;
    m_pServer->sendAll(msgWithPrefix.str(), false, m_id, m_status, m_deskID);

    //TODO: 这个返回值没意义，可以改成void
    return true;
}

bool Calc24Session::processCmd(const std::string& package) {
    //!ready 表示已经举手
    //!2 3 4 5 表示对24点游戏的结果进行计算
    if (package.substr(0, 6) == "!ready") {
        m_status = SESSION_STATUS_READY;
        return true;
    }

    if (package == "!xyz\n") {
        //假设是正确的答案
        //告诉当前玩家你的结果是正确的，结束游戏
        std::string rightgAnwser = "Congratulations! Your answer is right! Game Over!\n";
        m_spConn->send(rightgAnwser);
        //告诉其他玩家，已经有玩家算出结果，结束游戏

        std::string notifyOthersGameOverMsg = "One player has already given the right answer! Game Over!\n";
        m_pServer->sendAll(notifyOthersGameOverMsg, false, m_id, m_status, m_deskID);

        //让所有当前桌子上的玩家回到空闲状态
        m_pServer->resetSessionToIdleByDeskID(m_deskID);

    } else {
        //错误的答案
        std::string wrongAnwser = "Your answer is wrong, please try again.\n";
        m_spConn->send(wrongAnwser);
    }


    return false;
}