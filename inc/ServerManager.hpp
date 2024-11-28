#pragma once

#include "Webserv.hpp"
#include "Client.hpp"
#include "Response.hpp"

class Server;

class ServerManager
{
private:
    std::map<int, Server>   _server_map;
    std::map<int, Client>   _client_map;
    int                     _epoll_fd;

// Private member functions
    void    _acceptNewConnection(int fd);
    void    _closeConnection(int fd);
    void    _checkTimeout();
    void    _readRequest(Client &client);
    void    _sendResponse(Client &client);

public:
// Constructor
    ServerManager();

// Deconstructor
    ~ServerManager();

// Member functions
    void    setup(std::string config);
    void    boot();

};
