#pragma once

#include "Webserv.hpp"

class Socket
{
private:
    in_addr_t                           _host;
    uint16_t                            _port;
    int                                 _socket_fd;
    struct sockaddr_in                  _socket_address;

public:
// Constructor
    Socket();

// Deconstructor
    ~Socket();

// Getters
    in_addr_t           getHost() const;
    uint16_t            getPort() const;
    int                 getSocketFd() const;
    struct sockaddr_in  getSocketAddress() const;

// Setters
    void                setPort(uint16_t port);
    void                setHost(in_addr_t host);

// Member functions
    void                setup();
    void                startListening();
    int                 acceptConnection(); 

};
