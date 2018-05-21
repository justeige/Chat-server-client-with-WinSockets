#include <iostream>

#include "Server.h"
#include "Client.h"
#include "Common.h"

// external libs
#pragma comment(lib, "ws2_32.lib")

#define IS_SERVER

int main(int argc, char** argv)
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    try {

#if defined(IS_SERVER)

        Server server(DefaultPort);
        server.listen();
        server.shutdown();

#else
        Client client(DefaultPort);
        while (!client.connect(LocalServer)) {
            std::cout << "Try to reconnect...\n";
            Sleep(2000);
        }
        client.listen();
        client.send(std::cin);
#endif

    }
    catch (const std::exception& ex) {
        std::cout << ex.what() << '\n';
    }
    catch (...) {
        std::cout << "An unexpected exception happend!\n";
    }

    return EXIT_SUCCESS;
}