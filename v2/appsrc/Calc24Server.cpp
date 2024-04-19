#include "Calc24Server.h"

#include <iostream>

#include "Calc24Session.h"

bool Calc24Server::init(int32_t threadNum, const std::string& ip/* = ""*/, uint16_t port/* = 8888*/) {
    //m_tcpServer.setConnectedCallback([](std::shared_ptr<TCPConnection>& spConn) -> void {

    //    });

    m_tcpServer.setConnectedCallback(std::bind(&Calc24Server::onConnected, this, std::placeholders::_1));
    //m_tcpServer.setDisconnectedCallback(std::bind(&Calc24Server::onDisconnected, this, std::placeholders::_1));

    if (!m_tcpServer.init(1, ip, port)) {
        return false;
    }

    m_checkHandupTimerID = m_tcpServer.getBaseEventLoop().addTimer(1000, true, 0, [this](int64_t timerID) -> void {
        static constexpr int REQUIRED_MIN_READY_COUNT = 2;

        int readySessionCount = 0;
        //每隔1秒检测是否有足够数量的玩家已经准备好
        for (auto& iter : m_sessions) {
            if (iter.second->isReady()) {
                readySessionCount++;
                //iter.second->initCards();
            }
        }

        if (readySessionCount < REQUIRED_MIN_READY_COUNT) {
            return;
        }

        std::string newCards;
        int64_t deskID;
        generateCards(newCards, deskID);

        for (auto& iter : m_sessions) {
            if (iter.second->isReady()) {
                iter.second->sendCards(newCards, deskID);
            }
        }

        });


    m_tcpServer.start();

    return true;
}

void Calc24Server::uninit() {
    if (m_checkHandupTimerID > 0) {
        m_tcpServer.getBaseEventLoop().removeTimer(m_checkHandupTimerID);
    }

    m_tcpServer.uninit();
}

void Calc24Server::sendAll(const std::string& msg, bool includeSelf, int32_t id, int status, int64_t deskID) {
    for (const auto& iter : m_sessions) {
        if (!includeSelf) {
            if (iter.second->getID() == id) {
                continue;
            }
        }

        if (iter.second->getStatus() != status || iter.second->getDeskID() != deskID)
            continue;

        //iter指向的std::pair<int32_t, std::shared_ptr<Calc24Session>>
        //iter.second指向std::shared_ptr<Calc24Session>
        iter.second->sendMsg(msg);
    }
}

void Calc24Server::resetSessionToIdleByDeskID(int64_t deskID) {
    for (const auto& iter : m_sessions) {
        if (iter.second->getDeskID() != deskID)
            continue;

        //iter指向的std::pair<int32_t, std::shared_ptr<Calc24Session>>
        //iter.second指向std::shared_ptr<Calc24Session>
        iter.second->resetToIdle();
    }
}

void Calc24Server::onConnected(std::shared_ptr<TCPConnection>& spConn) {
    //删除无效的Calc24Session
    m_pendingToDeleteSessions.clear();

    auto spCalc24Session = std::make_shared<Calc24Session>(this, std::move(spConn));
    spCalc24Session->sendWelcomeMsg();

    spCalc24Session->notifyUserToBeReady();

    //std::pair<int, std::shared_ptr<Calc24Session>>
    m_sessions.emplace(spCalc24Session->getID(), std::move(spCalc24Session));
}

void Calc24Server::generateCards(std::string& newCards, int64_t& deskID) {
    static constexpr char allCards[] = { 'A', '2', '3', '4', '5', '6', '7', '8', '9', 'X', 'J', 'Q', 'K', 'w', 'W' };
    static constexpr int allCardsCount = sizeof(allCards) / sizeof(allCards[0]);

    int index1 = rand() % allCardsCount;
    int index2 = rand() % allCardsCount;
    int index3 = rand() % allCardsCount;
    int index4 = rand() % allCardsCount;

    char szNewCards[24];
    sprintf(szNewCards, "Your cards is: %c %c %c %c\n",
        allCards[index1],
        allCards[index2],
        allCards[index3],
        allCards[index4]);

    newCards = szNewCards;

    deskID = Calc24Server::generateDeskID();
}

int64_t Calc24Server::generateDeskID() {
    static std::atomic<int64_t> m_baseID{ 0 };

    int64_t deskID = ++m_baseID;
    return deskID;
}

void Calc24Server::onDisconnected(int32_t id) {
    auto iter = m_sessions.find(id);
    if (iter != m_sessions.end()) {
        auto pendingToDeleteSession = iter->second;
        m_pendingToDeleteSessions.push_back(std::move(pendingToDeleteSession));

        m_sessions.erase(iter);

        std::cout << "Calc24Server::onDisconnected" << std::endl;
    }
}