#include "../inc/Socket.hpp"

// =============   Constructor   ============= //
Socket::Socket()
{
    _fd = 0;
    _port = 0;
    _host = 0;
}

// ============   Deconstructor   ============ //
Socket::~Socket()
{
}

// ==============   Getters   ================ //
in_addr_t Socket::getHost() const
{
    return _host;
}

uint16_t Socket::getPort() const
{
    return _port;
}

int Socket::getSocketFd() const
{
    return _fd;
}

struct sockaddr_in Socket::getSocketAddress() const
{
    return _addr;
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
setting an fd into non-blocking mode
    - on success, zero is returned
    - on error, -1 is returned, and errno is set to indicate the error.
*/
static int    setNonBlocking(int fd)
{
    // gets the current flags for the file descriptor
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return -1;

    // sets the flags to include O_NONBLOCK
    flags |= O_NONBLOCK;

    // sets the new flags to the file descriptor
    if (fcntl(fd, F_SETFL, flags) == -1)
        return -1;
    return 0;
}

// ==========   Member functions   =========== //
/*
setting up an non blocking TCP socket for listening for new connections:
    - creating the socket
    - setting the socket to reuse ports
    - binding an address to the socket
    - setting the socket in non blocking mode
    - on success, zero is returned
    - on error, -1 is returned, and errno is set to indicate the error.
*/
int    Socket::setup()
{
    // creating the socket
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd < 0)
        return -1;

    // setting the socket to reuse ports
    const int opt = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        return -1;

    // binding an address to the socket
    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(_port);
    _addr.sin_addr.s_addr = htonl(_host);
    std::memset(_addr.sin_zero, '\0', sizeof(_addr.sin_zero));
    if (bind(_fd, (struct sockaddr *)&_addr, sizeof(_addr)) < 0)
        return -1;

    // setting the socket in non blocking mode
    if (setNonBlocking(_fd) < 0)
        return -1;
    return 0;
}

/*
starting listening of the socket for incoming connections
    - on success, zero is returned
    - on error, -1 is returned, and errno is set to indicate the error.
*/
int    Socket::startListening()
{
    if (listen(_fd, BACKLOG) < 0)
        return -1;
    return 0;
}

/*
accepting new incomming connection on the socket
    - setting the new socket for the connection in non blocking mode
    - on success, the fd of the new socket is returned
    - on error, -1 is returned, and errno is set to indicate the error
*/
int    Socket::acceptConnection()
{
    int new_socket;
    int addrlen = sizeof(_addr);

    if ((new_socket = accept(_fd, (struct sockaddr *)&_addr, (socklen_t*)&addrlen)) < 0)
        return -1;
    if (setNonBlocking(_fd) < 0)
        return -1;
    return new_socket;
}
