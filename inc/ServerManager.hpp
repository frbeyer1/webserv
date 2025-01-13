#pragma once

#include "Webserv.hpp"
#include "Client.hpp"
#include "Response.hpp"

class ServerManager
{
private:
    std::vector<ServerBlock>    _server_blocks;
    std::map<int, Socket>       _socket_map;
    std::map<int, Client>       _client_map;
    int                         _epoll_fd;

// Private member functions
    void    _acceptNewConnection(int fd);
    void    _closeConnection(int fd);
    void    _checkTimeout();
    void    _readRequest(Client &client);
    void    _sendResponse(Client &client);
    void    _findDefaultServer(Client &client);

public:
// Constructor
    ServerManager();

// Deconstructor
    ~ServerManager();

// Member functions
    void    setup(std::string config);
    void    boot();

};
