#include "Client.h"
#include "Common.h"

// --------------------------------------------------------------
Client::Client(int port) : m_port(port), m_socket(INVALID_SOCKET)
// --------------------------------------------------------------
{
    ZeroMemory(&m_address, sizeof(m_address));

    // init winsock
    WSADATA data = {};
    WORD requestedVersion = MAKEWORD(2, 2);
    int wsResult = WSAStartup(requestedVersion, &data);
    if (wsResult != 0) {
        throw WsaException("Failed to init Win-Sockets!");
    }
}

// --------------------------------------------------------------
Client::~Client()
// --------------------------------------------------------------
{
    closesocket(m_socket);
    WSACleanup();
}

// --------------------------------------------------------------
bool Client::connect(std::string ip)
// --------------------------------------------------------------
{
    // fill in a hint structure
    m_address = {};
    m_address.sin_family = AF_INET;
    m_address.sin_port = htons(m_port); // host-to-network-short
    inet_pton(AF_INET, ip.c_str(), &m_address.sin_addr);

    // create socket
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket == INVALID_SOCKET) {
        throw WsaException("Can't create socket");
    }

    // connect to server
    int result = ::connect(m_socket, (sockaddr*)&m_address, sizeof(m_address));
    if (result == SOCKET_ERROR) {
        std::cerr << "Can't connect to server, Err=" << WSAGetLastError() << '\n';
        return false;
    }

    // client should be connected to the server
    return true;
}

// --------------------------------------------------------------
void Client::listen()
// --------------------------------------------------------------
{
    // wait for a welcome message of the server
    std::array<char, 512> answer{};
    ::recv(m_socket, answer.data(), answer.size(), NULL);
    std::cout << "From Server: " << answer.data() << '\n';

    m_listener = std::thread(Listener{ m_socket });
}

// --------------------------------------------------------------
void Client::send(std::istream & in)
// --------------------------------------------------------------
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

// --------------------------------------------------------------
void Client::send(Message const & msg)
// --------------------------------------------------------------
{
    send(msg.length());
    ::send(m_socket, msg.body(), msg.length(), NULL);
}

// --------------------------------------------------------------
bool Client::send(int value)
// --------------------------------------------------------------
{
    return ::send(m_socket, (char*)&value, sizeof(int), NULL) != SOCKET_ERROR;
}

// --------------------------------------------------------------
bool Client::get(int & value)
// --------------------------------------------------------------
{
    return ::recv(m_socket, (char*)&value, sizeof(int), NULL) != SOCKET_ERROR;
}


// ---------------------------------------------------------------
// internal message handler class
// ---------------------------------------------------------------

Client::Listener::Listener(SOCKET& s)
    : m_socket(s) {}


// ---------------------------------------------------------------
void Client::Listener::operator()() const
// ---------------------------------------------------------------
{
    // receive first the message length, than the real message
    int msgLength;
    forever {

        if (SOCKET_ERROR == ::recv(m_socket, (char*)&msgLength, sizeof(int), NULL)) {
            break;
        }

        std::vector<char> buffer(msgLength);

        if (SOCKET_ERROR == ::recv(m_socket, &buffer[0], msgLength, NULL)) {
            break;
        }
        std::cout << buffer << '\n'; // not threadsafe!
    }
    std::cout << "lost connection to the server!\n";
    closesocket(m_socket);
}