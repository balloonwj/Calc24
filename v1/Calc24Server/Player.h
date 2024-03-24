#pragma once

#include <memory>
#include <string>

#define NO_PLAYER_ON_SEAT -1

class Player;

struct Desk {
    int id;
    std::weak_ptr<Player>  spPlayer1;
    std::weak_ptr<Player>  spPlayer2;
    std::weak_ptr<Player>  spPlayer3;

    std::string sendCards;
};

class Player final {
public:
    Player(int clientfd);
    ~Player();

    //TODO：拷贝构造函数和move构造

    int getClientfd() const;

    void setDesk(const std::shared_ptr<Desk>& spDesk);

    void setReady(bool ready);
    bool getReady() const;

    //是否已经发过牌
    bool isCardsSent() const;

    bool sendWelcomeMsg();
    bool sendCards();

    bool sendMsgToClient(const std::string& msg);

    bool recvData();
    void handleClientMsg(std::string& currentMsg);

private:
    const int               m_clientfd;
    //玩家所在桌是否人满是否发牌了，true表示已经可以发牌，false表示玩家数量不够，不能发牌
    bool                    m_ready{ false };
    bool                    m_cardsSent{ false };

    std::string             m_recvBuf;
    std::weak_ptr<Desk>     m_spDesk;
};