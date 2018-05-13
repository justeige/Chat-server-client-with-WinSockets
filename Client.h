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
#include <thread>

#include "Message.h"


class Client {
public:

    Client(int port);
    ~Client();

    void init();
    void connect(std::string ip);
    void listen();
    void send(std::istream& in);

protected:

    int m_port;
    SOCKET m_socket;
    sockaddr_in m_address;
    std::thread m_listener;

    void send(Message const& msg);
    bool send(int value);
    bool get(int& value);

    // functor that will listen for incoming messages
    class Listener final {
    public:
        Listener(SOCKET& s);
        void operator()() const;

    private:
        SOCKET & m_socket;
    };

};
