#include "../inc/Server.hpp"

// =============   Constructor   ============= //
Server::Server()
{
    _ip = DEFAULT_HOST;
    try
    {
        _host = ipStringToNumeric(DEFAULT_HOST);
    }
    catch(const std::exception& e)
    {
        std::ostringstream oss;
        oss << "Webserv header misconfigured: DEFAULT_HOST: ip invalid: " << e.what();
        Logger::log(RED, ERROR, oss.str());
        exit(EXIT_FAILURE);
    }
    _port = DEFAULT_PORT;
    _server_name = DEFAULT_NAME;
    _root = DEFAULT_ROOT;
    _client_max_body_size = DEFAULT_CLIENT_MAX_BODY_SIZE;
    _index = DEFAULT_INDEX;
    // _error_pages = ;
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

std::string Server::getIp() const
{
    return (_ip);
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

std::map<int, std::string>& Server::getErrorPages()
{
    return (_error_pages);
}

struct sockaddr_in  Server::getSocketAddress() const
{
    return (_socket_address);
}

// ==============   Setters   ================ //
void    Server::setRoot(std::string root)
{
    _root = root;
}

void    Server::setPort(uint16_t port)
{
    _port = port;
}

void    Server::setHost(in_addr_t host)
{
    _host = host;
}

void    Server::setIp(std::string ip)
{
    _ip = ip;
}

void    Server::setServerName(std::string server_name)
{
    _server_name = server_name;
}

void    Server::setClientMaxBodySize(size_t client_max_body_size)
{
    _client_max_body_size = client_max_body_size;
}

void    Server::setErrorPage(int status_code, std::string page_path)
{
    _error_pages.insert(std::pair<int, std::string>(status_code, page_path));
}

void    Server::setIndex(std::string index)
{
    _index = index;
}

void    Server::setLocation(std::string path, location_t location)
{
    _locations.insert(std::make_pair(path, location));
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
        Logger::log(RED, ERROR, "could not get flags from file descriptor");
        exit(EXIT_FAILURE);
    }
    // Set the flags to include O_NONBLOCK
    flags |= O_NONBLOCK;
    // Set the new flags for the file descriptor
    if (fcntl(fd, F_SETFL, flags) == -1)
    {
        Logger::log(RED, ERROR, "could not set flags to file descriptor");
        exit(EXIT_FAILURE);
    }
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
    std::memset(_socket_address.sin_zero, '\0', sizeof(_socket_address.sin_zero));
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
    setNonBlocking(new_socket);
    return (new_socket);
}
