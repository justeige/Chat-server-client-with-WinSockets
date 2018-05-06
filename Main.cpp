#include <iostream>

#include "Server.h"
#include "Client.h"

// external libs
#pragma comment(lib, "ws2_32.lib")

//#define IS_SERVER

int main(int argc, char** argv)
{
#if defined(IS_SERVER)
    Server server(54000);
    server.init();
    server.listen();
    server.wait();
    server.shutdown();
#else
    Client client(54000);
    client.init();
    client.connect(LocalServer);
    client.listen();
#endif
    return EXIT_SUCCESS;
}