#include "Calc24Session.h"

#include <functional>
#include <iostream>

#include "Calc24Server.h"

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
        if (recvBuf.remaining() <= sizeof(MsgHeader)) {
            return;
        }

        MsgHeader header;
        recvBuf.peek(reinterpret_cast<char*>(&header), sizeof(MsgHeader));

        if (header.packageSize <= recvBuf.remaining()) {
            if (!decodePackage(recvBuf, header)) {
                return;
            }
        }
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

void Calc24Session::sendWelcomeMsg() {
    const std::string& welcomeMsg = "Welcome to Calc24 Game.";
    m_spConn->send(welcomeMsg);
}

int Calc24Session::generateID() {
    static std::atomic<int32_t> m_baseID{ 0 };

    int32_t resultID = ++m_baseID;
    return resultID;
}

bool Calc24Session::decodePackage(ByteBuffer& recvBuf, const MsgHeader& header) {
    if (static_cast<MsgType>(header.msgType) == MsgType::MsgTypeWelcome) {
        //WelcomeMsg welcomeMsg;
        //recvBuf.retrieve(reinterpret_cast<char*>(&welcomeMsg), sizeof(welcomeMsg));

        //处理welcomeMsg

        return true;
    }

    return false;
}