#pragma once

#include "Webserv.hpp"

class   Server
{
private:
    int                 _fd;
    struct sockaddr_in  _address;
    int                 _reuseable_ports;

public:
// Constructor
    Server();

// Deconstructor
    ~Server();

// Member functions
    void    launch();

};
