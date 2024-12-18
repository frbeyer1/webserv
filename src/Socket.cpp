#include "../inc/Socket.hpp"

// =============   Constructor   ============= //
Socket::Socket()
{
}

// ============   Deconstructor   ============ //
Socket::~Socket()
{
}

// ==============   Getters   ================ //
in_addr_t Socket::getHost() const
{
    return (_host);
}

uint16_t Socket::getPort() const
{
    return (_port);
}

int Socket::getSocketFd() const
{
    return (_socket_fd);
}

struct sockaddr_in Socket::getSocketAddress() const
{
    return (_socket_address);
}

// ==============   Setters   ================ //
void    Socket::setPort(uint16_t port)
{
    _port = port;
}

void    Socket::setHost(in_addr_t host)
{
    _host = host;
}

// ================   Utils   ================ //
/*
function to set an fd into non-blocking mode
*/
static void    setNonBlocking(int fd)
{
    // Get the current flags for the file descriptor
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        Logger::log(RED, ERROR, "Could not get flags from file descriptor: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    // Set the flags to include O_NONBLOCK
    flags |= O_NONBLOCK;
    // Set the new flags for the file descriptor
    if (fcntl(fd, F_SETFL, flags) == -1)
    {
        Logger::log(RED, ERROR, "Could not set flags to file descriptor: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

// ==========   Member functions   =========== //
/*
function to create and setup an tcp socket
*/
void    Socket::setup()
{
    // 1. create the socket
    _socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket_fd < 0)
    {
        Logger::log(RED, ERROR, "Could not set up socket: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    // 2. sets the socket to reuse ports
    const int opt = 1;
    if (setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        Logger::log(RED, ERROR, "Setsockopt (SO_REUSEADDR) failed: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    // 3. setup and bind the address to the socket
    _socket_address.sin_family = AF_INET;
    _socket_address.sin_port = htons(_port);
    _socket_address.sin_addr.s_addr = htonl(_host);
    std::memset(_socket_address.sin_zero, '\0', sizeof(_socket_address.sin_zero));
    if (bind(_socket_fd, (struct sockaddr *)&_socket_address, sizeof(_socket_address)) < 0)
    {
        Logger::log(RED, ERROR, "Could not bind socket: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    setNonBlocking(_socket_fd);
}

/*
start to listen for incoming connections
*/
void    Socket::startListening()
{
    if (listen(_socket_fd, BACKLOG) < 0)
    {
        Logger::log(RED, ERROR, "Socket could not listen: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    Logger::log(GREY, DEBUG, "fd[%i] started listening", _socket_fd);
}

/*
accept new connection
*/
int    Socket::acceptConnection()
{
    int new_socket;
    int addrlen = sizeof(_socket_address);

    if ((new_socket = accept(_socket_fd, (struct sockaddr *)&_socket_address, (socklen_t*)&addrlen)) < 0)
    {
        Logger::log(RED, ERROR, "Socket could not accept connection: %s", strerror(errno));
        exit(EXIT_FAILURE);        
    }
    setNonBlocking(new_socket);
    return (new_socket);
}
