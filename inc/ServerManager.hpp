#pragma once

#include "Webserv.hpp"

class Server;

class Client;

class ServerManager
{
private:
    std::vector<Server>       _servers;
    std::map<int, Server>     _server_map;
    std::map<int, Client>     _client_map;
    int                       _epoll_fd;

// Private member functions
    void    _acceptNewConnection(int fd);
    void    _closeConnection(int fd);
    void    _checkTimeout();
    void    _readRequest(int fd);
    void    _sendResponse(int fd);

public:
// Constructor
    ServerManager();

// Deconstructor
    ~ServerManager();

// Member functions
    void    configure(std::string config);
    void    setup();
    void    boot();

};
