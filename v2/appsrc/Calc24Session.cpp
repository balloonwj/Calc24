#include "Calc24Session.h"

#include <functional>

Calc24Session::Calc24Session(std::shared_ptr<TCPConnection>&& spConn) : m_spConn(std::move(spConn)) {
    m_spConn->setReadCallback(std::bind(&Calc24Session::onRead, this, std::placeholders::_1));
    m_spConn->setWriteCallback(std::bind(&Calc24Session::onWrite, this));
    m_spConn->setCloseCallback(std::bind(&Calc24Session::onClose, this));
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

}

void Calc24Session::sendWelcomeMsg() {
    const std::string& welcomeMsg = "Welcome to Calc24 Game.";
    m_spConn->send(welcomeMsg);
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