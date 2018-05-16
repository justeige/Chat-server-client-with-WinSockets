#include <iostream>

#include "Server.h"
#include "Client.h"
#include "Common.h"

// external libs
#pragma comment(lib, "ws2_32.lib")

#define IS_SERVER

int main(int argc, char** argv)
{
#if defined(IS_SERVER)
    Server server(DefaultPort);
    server.init();
    server.listen();
    server.shutdown();
#else
    Client client(DefaultPort);
    client.init();
    client.connect(LocalServer);
    client.listen();
    client.send(std::cin);
#endif
    return EXIT_SUCCESS;
}