#pragma once

//TODO: 头文件包含方式待修改
#include "TCPConnection.h"

#include <atomic>
#include <memory>

#include "Calc24Protocol.h"

class Calc24Server;

#define SESSION_STATUS_CONNECTED   0 //未举手状态
#define SESSION_STATUS_HANDUP      1 //举手状态
#define SESSION_STATUS_CARDSINITED 2 //发完牌，游戏状态

enum class DecodePackageResult {
    DecodePackageSuccess,
    DecodePackageFailed,
    DecodePackageWantMoreData
};

class Calc24Session : private std::enable_shared_from_this<Calc24Session> {
public:
    Calc24Session(Calc24Server* pServer, std::shared_ptr<TCPConnection>&& spConn);
    ~Calc24Session();

    //TODO: 为啥返回值是bool类型
    void onRead(ByteBuffer& recvBuf);
    void onWrite();

    void onClose();

    int32_t getID() const {
        return m_id;
    }

    void sendMsg(const std::string& msg);

    //业务代码
    void sendWelcomeMsg();

    void notifyUserHandup();

    //给玩家发牌
    void initCards();

    void forceClose();

    bool isHandup() const {
        return m_status == SESSION_STATUS_HANDUP;
    }

private:
    static int generateID();

private:
    DecodePackageResult decodePackage(ByteBuffer& recvBuf);

    bool processPackage(const std::string& package);

    bool processChatMsg(const std::string& package);
    bool processCmd(const std::string& package);

private:
    int32_t                             m_id;
    Calc24Server* m_pServer;
    std::shared_ptr<TCPConnection>      m_spConn;

    std::atomic<int8_t>                 m_status{ SESSION_STATUS_CONNECTED };
};
