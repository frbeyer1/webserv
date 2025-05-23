#pragma once

#include "Webserv.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "CgiHandler.hpp"

struct Client
{
    struct sockaddr_in  client_address;
    int                 client_fd;
    time_t              last_msg_time;
    Request             request;
    Response            response;
    Socket              *socket;
    e_data              *epoll_data;
};
