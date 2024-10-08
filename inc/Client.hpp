#pragma once

#include "Webserv.hpp"

class Client
{
private:
    int             _client_fd;
    time_t          _last_msg_time;

public:
// Constructor
    Client();

// Deconstructor
    ~Client();

// Getter
    int     getClientFd() const;
    time_t  getLastMsgTime() const;

// Setter
    void    setClientFd(int fd);
    void    setLastMsgTime(time_t time);

};