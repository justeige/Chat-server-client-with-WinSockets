#pragma once
// -------------------------------------
// simple winsocket client
// -------------------------------------


#include <WS2tcpip.h> // windows tcp
#include <assert.h>
#include <iostream>
#include <memory>

const std::string LocalServer = "127.0.0.1";

class Client {
public:
    Client(int port) : m_port(port), m_socket(INVALID_SOCKET)
    {
        ZeroMemory(&m_address, sizeof(m_address));
    }

    ~Client()
    {
        closesocket(m_socket);
        WSACleanup();
    }

    void init()
    {
        // init winsock
        WSADATA data = {};
        WORD requestedVersion = MAKEWORD(2, 2);
        int wsResult = WSAStartup(requestedVersion, &data);
        if (wsResult != 0) {
            std::exit(EXIT_FAILURE);
        }
    }

    void connect(std::string ip)
    {
        // fill in a hint structure
        m_address = {};
        m_address.sin_family = AF_INET;
        m_address.sin_port = htons(m_port); // host-to-network-short
        inet_pton(AF_INET, ip.c_str(), &m_address.sin_addr);

        // create socket
        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (m_socket == INVALID_SOCKET) {
            std::cerr << "Can't create socket, Err=" << WSAGetLastError() << '\n';
            std::exit(EXIT_FAILURE);
        }

        // connect to server
        int result = ::connect(m_socket, (sockaddr*)&m_address, sizeof(m_address));
        if (result == SOCKET_ERROR) {
            std::cerr << "Can't connect to server, Err=" << WSAGetLastError() << '\n';
            std::exit(EXIT_FAILURE);
        }
    }

protected:
    int m_port;
    SOCKET m_socket;
    sockaddr_in m_address;
};
