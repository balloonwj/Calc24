#pragma once

//TODO: 头文件包含方式待修改
#include "TCPConnection.h"

#include "Calc24Protocol.h"

class Calc24Session {
public:
    Calc24Session(std::shared_ptr<TCPConnection>&& spConn);
    ~Calc24Session() = default;

    //TODO: 为啥返回值是bool类型
    void onRead(ByteBuffer& recvBuf);
    void onWrite();

    void onClose();

    //业务代码
    void sendWelcomeMsg();

private:
    bool decodePackage(ByteBuffer& recvBuf, const MsgHeader& header);

private:
    std::shared_ptr<TCPConnection>      m_spConn;
};
