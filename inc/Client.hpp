#pragma once

#include "Webserv.hpp"
#include "HttpRequest.hpp"
#include "Response.hpp"

class Client
{
private:
    struct sockaddr_in  _client_address;
    int                 _client_fd;
    time_t              _last_msg_time;

public:
    Server              server;
    HttpRequest         request;
    
// Default Constructor
    Client();

// Constructor
    Client(Server &s);

// Copy Constructor
    Client(const Client &rhs);

// Deconstructor
    ~Client();

// Getter
    int                 getClientFd() const;
    time_t              getLastMsgTime() const;
    struct sockaddr_in  getClientAddress() const;

// Setter
    void                setClientFd(int fd);
    void                setLastMsgTime(time_t time);
    void                setClientAddress(struct sockaddr_in client_address);

};