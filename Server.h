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
#include <mutex>

class Server {

    class  ClientHandler;
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
    Server(int port);
    ~Server();

    void init();
    void listen();
    void shutdown();

protected:
    int m_port = 0;
    SOCKET m_socket;
    sockaddr_in m_address;
    Clients m_clients;
    Threads m_clientHandler;
    std::mutex m_lock;

    // abstraction for connected clients
    struct Connection {
        SOCKET socket = INVALID_SOCKET;
        int id = -1;
        bool isActive = false;
    };

    // functor that will listen for incoming messages and send them to other clients
    class ClientHandler final {
    public:
        ClientHandler(Connection c, Clients& other);
        void operator()() const;

    private:
        Connection m_connection;
        Clients& m_otherClients;
    };

    // member functions
    void disconnectClient(int id);
};