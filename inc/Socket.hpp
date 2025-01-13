#pragma once

#include "Webserv.hpp"

class Socket
{
private:
    int                         _fd;
    uint16_t                    _port;
    in_addr_t                   _host;
    struct sockaddr_in          _addr;

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
    int                 setup();
    int                 startListening();
    int                 acceptConnection(); 

};
