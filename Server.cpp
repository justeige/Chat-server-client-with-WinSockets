#include "Server.h"
#include "Common.h"

// ---------------------------------------------------------------
Server::Server(int port) : m_port(port), m_socket(INVALID_SOCKET)
// ---------------------------------------------------------------
{
    ZeroMemory(&m_address, sizeof(m_address));

    // init Winsock dll
    WSADATA wsData = {};
    if (0 != WSAStartup(MAKEWORD(2, 2), &wsData)) {
        throw WsaException("Winsock init failed");
    }

    // create a socket for listening
    m_socket = socket(AFS_IPv4, SOCK_STREAM, 0); // unspecified protokol = 0
    if (m_socket == INVALID_SOCKET) {
        throw WsaException("Creating listening socket failed");
    }

    // create an address for listening from ip/port
    m_address.sin_family = AFS_IPv4;
    m_address.sin_port = htons(m_port); // host-to-network-short
    m_address.sin_addr.S_un.S_addr = INADDR_ANY;

    // bind address to the socket
    if (SOCKET_ERROR == bind(m_socket, (sockaddr*)&m_address, sizeof(m_address))) {
        throw WsaException("Binding server address to socket failed");
    }

    // tell winsock the sock is for listening
    if (SOCKET_ERROR == ::listen(m_socket, SOMAXCONN)) {
        throw WsaException("Starting socket as a listener failed");
    }
}

// ---------------------------------------------------------------
Server::~Server()
// ---------------------------------------------------------------
{
    closesocket(m_socket);
    WSACleanup();
}

// ---------------------------------------------------------------
// wait for connections, then spawn a client handler for each
// ---------------------------------------------------------------
void Server::listen()
// ---------------------------------------------------------------
{
    std::cout << "Server: Start to listen for clients\n";

    sockaddr_in client = {};
    int size = sizeof(client);

    forever {
        SOCKET newClient = ::accept(m_socket, (sockaddr*)&client, &size);
        if (newClient == INVALID_SOCKET) {
            throw WsaException("Binding a client socket failed!");
            return;
        }

        std::cout << "A client connected to the server!\n";

        // send a welcome message to the client to verify the connection
        char welcome[512] = "Welcome! You connected to the server!";
        ::send(newClient, welcome, sizeof(welcome), NULL);

        // client handler share the m_clients, so lock it up
        std::lock_guard<std::mutex> lock(m_lock);

        Connection connection = {};
        // try to reuse connection slots
        if (m_usableSlot > 0) {
            for (std::size_t i = 0; i < m_clients.size(); ++i) {
                if (!m_clients[i].isActive) {
                    m_clients[i].socket = newClient;
                    m_clients[i].isActive = true;
                    m_clients[i].id = i;
                    m_usableSlot--;
                    connection = m_clients[i];
                    break;
                }
            }
        }
        else {
            // no unused slots = create a complete new connection
            const int id = m_clients.size();
            connection = { newClient, id, true };
            m_clients.emplace_back(connection);
        }

        // spawn a handler for incoming messages and resending to other clients
        std::thread handler(ClientHandler{ connection, this });
        handler.detach();
    }
}

// ---------------------------------------------------------------
void Server::shutdown()
// ---------------------------------------------------------------
{
    auto result = ::shutdown(m_socket, SD_SEND);
    if (result == SOCKET_ERROR) {
        throw WsaException("Server shutdown failed!");
    }
}

// ---------------------------------------------------------------
void Server::disconnectClient(std::size_t id)
// ---------------------------------------------------------------
{
    // all client handler share the client list (I know its dangerous)
    // so this function has to lock that resource up
    std::lock_guard<std::mutex> lock(m_lock);

    // sanity check: don't disconnect already closed connections
    if (!m_clients[id].isActive) {
        assert(false && "tried to disconnect an already closed connection");
        return;
    }

    // close the socket
    m_clients[id].isActive = false;
    ::closesocket(m_clients[id].socket);

    // if the client that was disconnected was last, just free its memory
    // if it wasn't, mark the place in the vector as 'usable' and use it for another client
    if (id == m_clients.size() - 1) {
        m_clients.pop_back();

        // check if the connection-slots before are inactive aswell
        // -> only consecutive inactive connections are interesting
        // -> other slots will be filled later
        for (std::size_t i = m_clients.size() - 1; i >= 0 && m_clients.size() > 0; i--) {
            if (m_clients[i].isActive) {
                break;
            }

            m_clients.pop_back();
            m_usableSlot--;
        }
    }
    else {
        // can't be cleaned up right away
        m_usableSlot++;
    }
}
// ---------------------------------------------------------------
// internal client handler class
// ---------------------------------------------------------------

Server::ClientHandler::ClientHandler(Connection c, Server* parent)
    : m_connection(c), m_parent(parent)
{
}

// ---------------------------------------------------------------
void Server::ClientHandler::operator()() const
// ---------------------------------------------------------------
{
    // receive first the message length, than the real message
    int msgLength;
    forever {

        if (SOCKET_ERROR == ::recv(m_connection.socket, (char*)&msgLength, sizeof(msgLength), NULL)) {
            break; // might occur if the socket is closed
        }

        std::vector<char> buffer(msgLength);

        if (SOCKET_ERROR == ::recv(m_connection.socket, &buffer[0], msgLength, NULL)) {
            break; // might occur if the socket is closed
        }

        // parrot the message back to all other sockets
        for (auto client : m_parent->m_clients) {
            if (client.id == m_connection.id) { continue; } // don't 'echo' a client

            ::send(client.socket, (char*)&msgLength, sizeof(msgLength), NULL);
            ::send(client.socket, &buffer[0], msgLength, NULL);
        }
    }
    std::cout << "lost connection to client " << m_connection.id << "\n";
    m_parent->disconnectClient(m_connection.id);
}