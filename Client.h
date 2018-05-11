#pragma once
// -------------------------------------
// simple winsocket client
// -------------------------------------


#include <WS2tcpip.h> // windows tcp
#include <assert.h>
#include <iostream>
#include <memory>
#include <array>
#include <string>

#include "Message.h"

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

    void listen()
    {
        // wait for a welcome message of the server
        std::array<char, 512> answer{};
        ::recv(m_socket, answer.data(), answer.size(), NULL);
        std::cout << "From Server: " << answer.data() << '\n';
    }

    void send(std::istream& in)
    {
        // - take input from std::cin
        // - send the length of the input
        // - send the input as msg
        std::string message;
        for (;;) {
            std::getline(in, message);
            send({ message });
        }
    }

protected:

    int m_port;
    SOCKET m_socket;
    sockaddr_in m_address;

    void send(Message const& msg)
    {
        send(msg.length());
        ::send(m_socket, msg.body(), msg.length(), NULL);
    }

    bool send(int value)
    {
        return ::send(m_socket, (char*)&value, sizeof(int), NULL) != SOCKET_ERROR;
    }

    bool get(int& value)
    {
        return ::recv(m_socket, (char*)&value, sizeof(int), NULL) != SOCKET_ERROR;
    }
};
