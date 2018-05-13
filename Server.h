#pragma once
// -------------------------------------
// simple winsocket server
// -------------------------------------

#include <WS2tcpip.h> // windows tcp
#include <assert.h>
#include <iostream>
#include <memory>
#include <vector>
#include <array>
#include <thread>

class Server {

    // internal types
    enum AddressFamilySpecification {
        AFS_Unspecified = AF_UNSPEC,
        AFS_IPv4 = AF_INET,
        AFS_IPv6 = AF_INET6
    };

    using Sockets = std::vector<SOCKET>;
    using Threads     = std::vector<std::thread>;

public:
    Server(int port);
    ~Server();

    void init();
    void listen();
    void shutdown();

protected:
    int m_port = 0;
    SOCKET m_socket;
    sockaddr_in m_address;
    Sockets m_clients;
    Threads m_clientHandler;

    // functor that will listen for incoming messages and send them to other clients
    class ClientHandler final {
    public:
        ClientHandler(SOCKET& s, std::size_t id, Sockets& other);
        void operator()() const;

    private:
        SOCKET & m_socket;
        std::size_t m_id;
        Sockets& m_otherClients;
    };
};