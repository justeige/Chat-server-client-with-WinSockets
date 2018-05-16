#include "Server.h"
#include "Common.h"

// ---------------------------------------------------------------
Server::Server(int port) : m_port(port), m_socket(INVALID_SOCKET)
// ---------------------------------------------------------------
{
    ZeroMemory(&m_address, sizeof(m_address));
}

// ---------------------------------------------------------------
Server::~Server()
// ---------------------------------------------------------------
{
    closesocket(m_socket);
    WSACleanup();
}

// ---------------------------------------------------------------
void Server::init()
// ---------------------------------------------------------------
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

// ---------------------------------------------------------------
// wait for connections, then spawn a client handler for each
// ---------------------------------------------------------------
void Server::listen()
// ---------------------------------------------------------------
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

        const int id = m_clients.size();
        Connection connection = { newClient, id, true };
        m_clients.emplace_back(connection);
        m_clientHandler.emplace_back(ClientHandler{ connection, m_clients });
    }
}

// ---------------------------------------------------------------
void Server::shutdown()
// ---------------------------------------------------------------
{
    auto result = ::shutdown(m_socket, SD_SEND);
    if (result == SOCKET_ERROR) {
        std::cerr << "shutdown failed: " << WSAGetLastError() << '\n';
    }
}

// ---------------------------------------------------------------
void Server::disconnectClient(int id)
// ---------------------------------------------------------------
{
    // all client handler share the client list (I know its dangerous)
    // so this function has to lock that resource up
    std::lock_guard<std::mutex> lock(m_lock);

    // sanity check: don't disconnect already closed connections

}

// ---------------------------------------------------------------
// internal client handler class
// ---------------------------------------------------------------

Server::ClientHandler::ClientHandler(Connection c, Clients& other)
    : m_connection(c), m_otherClients(other)
{
}

// ---------------------------------------------------------------
void Server::ClientHandler::operator()() const
// ---------------------------------------------------------------
{
    // receive first the message length, than the real message
    int msgLength;
    forever {

        if (SOCKET_ERROR == ::recv(m_connection.socket, (char*)&msgLength, sizeof(int), NULL)) {
            break;
        }

        std::vector<char> buffer(msgLength);

        if (SOCKET_ERROR == ::recv(m_connection.socket, &buffer[0], msgLength, NULL)) {
            break;
        }

        // parrot the message back to all other sockets
        for (auto client : m_otherClients) {
            if (client.id == m_connection.id) { continue; } // don't 'echo' a client

            ::send(client.socket, (char*)&msgLength, sizeof(int), NULL);
            ::send(client.socket, &buffer[0], msgLength, NULL);
        }
    }
    std::cout << "lost connection to a client!\n";
    //closesocket(m_socket);
}