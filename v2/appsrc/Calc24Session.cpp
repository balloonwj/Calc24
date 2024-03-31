#include "Calc24Session.h"

#include <functional>
#include <iostream>
#include <sstream>

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

void Calc24Session::forceClose() {
    m_spConn->onClose();
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
    std::cout << "client[" << m_id << "] says: " << package << std::endl;

    std::ostringstream msgWithPrefix;
    msgWithPrefix << "client[";
    msgWithPrefix << m_id;
    msgWithPrefix << "] Says: ";
    msgWithPrefix << package;
    m_pServer->sendAll(msgWithPrefix.str(), false, m_id);

    return true;
}