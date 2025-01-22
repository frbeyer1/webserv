#pragma once

#include "Webserv.hpp"
#include "Client.hpp"
#include "Response.hpp"

struct e_data 
{
	int	fd;
	int client_fd;
};

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
    void    _readRequest(Client &client, struct epoll_event event);
    void    _sendResponse(Client &client, struct epoll_event event);
    void    _writeBodyToCgi(Client &client, int pipe_fd);
    void    _readCgiResponse(Client &client, int pipe_fd);
    void    _checkTimeout();

public:
// Constructor
    ServerManager();

// Deconstructor
    ~ServerManager();

// Member functions
    void    setup(std::string config);
    void    boot();

};
