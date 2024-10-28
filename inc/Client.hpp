#pragma once

#include "Webserv.hpp"

class Client
{
private:
    struct sockaddr_in  _client_address;
    int                 _client_fd;
    
    time_t              _last_msg_time;

public:
    HttpRequest         request;
    Server              &server;

// Constructor
    Client(Server &s);

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