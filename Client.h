#pragma once
// -------------------------------------
// simple winsocket client
// -------------------------------------


#include <WS2tcpip.h> // windows tcp
#include <assert.h>
#include <iostream>
#include <memory>


class Client {
public:
    Client(int port) : m_port(port)
    {

    }

    ~Client()
    {

    }

protected:
    int m_port;
    SOCKET m_socket;
};
