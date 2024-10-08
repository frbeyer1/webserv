#include "Server.hpp"

// =============   Constructor   ============= //
Server::Server()
{
    // _host = ;
    _port = DEFAULT_PORT;
    _server_name = DEFAULT_NAME;
    _root = DEFAULT_ROOT;
    _client_max_body_size = DEFAULT_CLIENT_MAX_BODY_SIZE;
    // _error_pages = ;
    // _locations = ;
    // _socket_address =
}

// ============   Deconstructor   ============ //
Server::~Server()
{
}

// ==============   Getters   ================ //
in_addr_t   Server::getHost() const
{
    return (_host);
}

uint16_t    Server::getPort() const
{
    return (_port);
}

int Server::getServerFd() const
{
    return (_server_fd);
}

std::string Server::getSeverName() const
{
    return (_server_name);
}

std::string Server::getRoot() const
{
    return (_root);
}

size_t  Server::getClientMaxBodySize() const
{
    return (_client_max_body_size);
}

// ==============   Setters   ================ //
void    Server::setRoot(std::string parameter)
{


}

void    Server::setListen(std::string parameter)
{
    

}

void    Server::setServerName(std::string parameter)
{
    

}

void    Server::setClientMaxBodySize(std::string parameter)
{
    

}

void    Server::setErrorPage(std::string parameter)
{

}

// ==========   Member functions   =========== //
/*
function to create and setup an tcp server socket
*/
void    Server::setup()
{
    // 1. create the socket
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd < 0)
    {
        Logger::log(RED, ERROR, "Could not set up socket");
        exit(EXIT_FAILURE);
    }
    // 2. sets the socket to reuse ports
    const int opt = 1;
    if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        Logger::log(RED, ERROR, "setsockopt (SO_REUSEADDR) failed");
        exit(EXIT_FAILURE);
    }
    // 3. setup and bind the address to the socket
    _socket_address.sin_family = AF_INET;
    _socket_address.sin_port = htons(_port);
    _socket_address.sin_addr.s_addr = htonl(_host);
    memset(_socket_address.sin_zero, '\0', sizeof(_socket_address.sin_zero));
    if (bind(_server_fd, (struct sockaddr *)&_socket_address, sizeof(_socket_address)) < 0)
    {
        Logger::log(RED, ERROR, "Could not bind socket");
        exit(EXIT_FAILURE);
    }
}

/*
start to listen for incoming connections
*/
void    Server::startListening()
{
    if (listen(_server_fd, BACKLOG) < 0)
    {
        Logger::log(RED, ERROR, "Socket could not listen");
        exit(EXIT_FAILURE);
    }
}

/*
accept new connection
*/
int    Server::acceptConnection()
{
    int new_socket;
    int addrlen = sizeof(_socket_address);

    if ((new_socket = accept(_server_fd, (struct sockaddr *)&_socket_address, (socklen_t*)&addrlen))<0)
    {
        Logger::log(RED, ERROR, "Socket could not accept connection");
        exit(EXIT_FAILURE);        
    }
    return (new_socket);
}

/*
function to set an fd into non-blocking mode
*/
// void    set_non_blocking(int fd)
// {
//     // Get the current flags for the file descriptor
//     int flags = fcntl(fd, F_GETFL, 0);
//     if (flags == -1)
//     {
//         std::cerr << "Error getting flags for file descriptor" << std::endl;
//         exit(EXIT_FAILURE);
//     }
//     // Set the flags to include O_NONBLOCK
//     flags |= O_NONBLOCK;
//     // Set the new flags for the file descriptor
//     if (fcntl(fd, F_SETFL, flags) == -1)
//     {
//         std::cerr << "Error setting flags for file descriptor" << std::endl;
//         exit(EXIT_FAILURE);
//     }
// }

// uint32_t ipStringToNumeric(const std::string& ip) {
//     std::stringstream ss(ip);
//     std::string segment;
//     uint32_t numericIp = 0;
//     int segmentCount = 0;

//     while (std::getline(ss, segment, '.')) {
//         if (segmentCount >= 4) {
//             throw std::invalid_argument("Invalid IP address format");
//         }
//         int segmentValue = std::stoi(segment);
//         if (segmentValue < 0 || segmentValue > 255) {
//             throw std::invalid_argument("IP address segments must be between 0 and 255");
//         }
//         numericIp = (numericIp << 8) | segmentValue; // Shift left by 8 and add segment
//         segmentCount++;
//     }

//     if (segmentCount != 4) {
//         throw std::invalid_argument("Invalid IP address format: must have 4 segments");
//     }
    
//     return numericIp;
// }

