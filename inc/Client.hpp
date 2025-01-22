#pragma once

#include "Webserv.hpp"
#include "Request.hpp"
#include "Response.hpp"

struct Client
{
    struct sockaddr_in  client_address;
    int                 client_fd;
    time_t              last_msg_time;
    Request             request;
    Response            response;
    Socket              *socket;
};
