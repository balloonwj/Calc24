// calc24v2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "Calc24Server.h"

int main()
{
    Calc24Server calc24Server;
    if (!calc24Server.init(5, "0.0.0.0", 8888)) {
        return 0;
    }

    return 1;
}

