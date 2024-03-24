// Calc24Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "TCPClient.h"

int main()
{
    TCPClient tcpClient;
    if (!tcpClient.init("127.0.0.1", 8888))
    {
        std::cout << "tcpClient.init failed" << std::endl;
        return 0;
    }

    return 1;
}