// Calc24Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "TCPServer.h"

int main()
{
    TCPServer tcpServer;
    if (!tcpServer.init("127.0.0.1", 8888))
    {
        std::cout << "tcpServer.init failed" << std::endl;
        return 0;
    }

    return 1;
}
