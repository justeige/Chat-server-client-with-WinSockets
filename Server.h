#pragma once
// -------------------------------------
// simple winsocket server
// -------------------------------------

#include <WS2tcpip.h> // windows tcp
#include <cassert>
#include <iostream>
#include <memory>
#include <vector>
#include <array>
#include <thread>
#include <mutex>

class Server {

    class  ClientHandler;
    friend ClientHandler;
    struct Connection;


    // internal types
    enum AddressFamilySpecification {
        AFS_Unspecified = AF_UNSPEC,
        AFS_IPv4 = AF_INET,
        AFS_IPv6 = AF_INET6
    };

    using Clients = std::vector<Connection>;
    using Threads = std::vector<std::thread>;

public:

    // functions might throw wsa-exceptions

    Server(int port);
    ~Server() noexcept;

    void listen();
    void shutdown();

private:
    int m_port = 0;
    SOCKET m_socket;
    sockaddr_in m_address;
    Clients m_clients;
    Threads m_clientHandler;
    std::mutex m_lock;
    int m_usableSlot = 0;

    // abstraction for connected clients
    struct Connection {
        SOCKET socket = INVALID_SOCKET;
        int id = -1;
        bool isActive = false;
    };

    // functor that will listen for incoming messages and send them to other clients
    class ClientHandler final {
    public:
        ClientHandler(Connection c, Server* parent);
        void operator()() const;

    private:
        Connection m_connection;
        Server* m_parent;
    };

    // member functions
    void disconnectClient(int id);
};