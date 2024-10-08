#pragma once

#include "Webserv.hpp"

class ServerManager
{
private:
    std::vector<Server>       _servers;
    std::map<int, Server>     _server_map;
    std::map<int, Client>     _client_map;
    int                       _epoll_fd;

public:
// Constructor
    ServerManager();

// Deconstructor
    ~ServerManager();

// Member functions
    void    configure(std::string config);
    void    setup();
    void    boot();
    void    acceptNewConnection(int fd);
    void    closeConnection(int fd);
    void    checkTimeout();

};
