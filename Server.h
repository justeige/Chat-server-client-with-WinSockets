#pragma once
// -------------------------------------
// simple winsocket server
// -------------------------------------

#include <WS2tcpip.h> // windows tcp
#include <assert.h>
#include <iostream>
#include <memory>
#include <vector>


enum AddressFamilySpecification {
    AFS_Unspecified = AF_UNSPEC,
    AFS_IPv4 = AF_INET,
    AFS_IPv6 = AF_INET6
};


class Server {

    // internal types
    using Connections = std::vector<SOCKET>;

public:
    Server(int port) : m_port(port), m_socket(INVALID_SOCKET)
    {
        ZeroMemory(&m_address, sizeof(m_address));
    }

    ~Server()
    {
        closesocket(m_socket);
        WSACleanup();
    }

    void init()
    {
        // init Winsock dll
        WSADATA wsData = {};
        if (0 != WSAStartup(MAKEWORD(2, 2), &wsData)) {
            std::cerr << "Winsock init failed\n";
            assert(false);
            std::exit(EXIT_FAILURE);
        }

        // create a socket for listening
        m_socket = socket(AFS_IPv4, SOCK_STREAM, 0); // unspecified protokol = 0
        if (m_socket == INVALID_SOCKET) {
            std::cerr << "Creating listening socket failed\n";
            assert(false);
            std::exit(EXIT_FAILURE);
        }

        // create an address for listening from ip/port
        m_address.sin_family = AFS_IPv4;
        m_address.sin_port = htons(m_port); // host-to-network-short
        m_address.sin_addr.S_un.S_addr = INADDR_ANY;

        // bind address to the socket
        if (SOCKET_ERROR == bind(m_socket, (sockaddr*)&m_address, sizeof(m_address))) {
            std::exit(EXIT_FAILURE);
        }

        // tell winsock the sock is for listening
        if (SOCKET_ERROR == ::listen(m_socket, SOMAXCONN)) {
            std::exit(EXIT_FAILURE);
        }
    }

    // wait for connections
    void listen()
    {
        sockaddr_in client = {};
        int size = sizeof(client);

        for (;;) {
            SOCKET newClient = ::accept(m_socket, (sockaddr*)&client, &size);
            if (newClient == INVALID_SOCKET) {
                std::cerr << "Client socket binding failed!\n";
                std::exit(EXIT_FAILURE);
            }

            std::cout << "A client connected to the server!\n";

            // send a welcome message to the client to verify the connection
            char welcome[512] = "Welcome! You connected to the server!";
            ::send(newClient, welcome, sizeof(welcome), NULL);

            m_clients.emplace_back(newClient);
        }
    }

    void wait()
    {
        for (;;) {}
    }

    void shutdown()
    {
        auto result = ::shutdown(m_socket, SD_SEND);
        if (result == SOCKET_ERROR) {
            std::cerr << "shutdown failed: " << WSAGetLastError() << '\n';
        }
    }

protected:
    int m_port = 0;
    SOCKET m_socket;
    sockaddr_in m_address;
    Connections m_clients;
};