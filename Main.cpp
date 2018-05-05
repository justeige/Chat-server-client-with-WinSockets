#include <iostream>

#include "Server.h"
#include "Client.h"

// external libs
#pragma comment(lib, "ws2_32.lib")

int main(int argc, char** argv)
{
    Server server(54000);
    server.init();
    server.listen();
    return EXIT_SUCCESS;
}