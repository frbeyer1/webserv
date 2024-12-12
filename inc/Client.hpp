#pragma once

#include "Webserv.hpp"
#include "Request.hpp"
#include "Response.hpp"

class Response;

struct Client
{
    struct sockaddr_in  _client_address;
    int                 _client_fd;
    time_t              _last_msg_time;
    Request             request;
    Response            response;
    Socket              *socket;
};
