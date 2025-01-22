#pragma once

#include "Webserv.hpp"

class Socket;

struct AllowedMethods
{
    bool                                allow_get;
    bool                                allow_delete;
    bool                                allow_post;
};

struct Location
{
    std::string                         alias;
    std::string                         index;
    std::string                         upload;
    std::string                         redirection;
    AllowedMethods                      allowed_methods;
    std::map<std::string, std::string>  cgi;
    bool                                autoindex;
};

struct ServerBlock
{
    std::vector<std::string>            server_names;
    in_addr_t                           host;
    uint16_t                            port;
    std::string                         ip;
    std::string                         root;
    size_t                              client_max_body_size;
    std::map<int, std::string>          error_pages;
    std::map<std::string, Location>     locations;
    Socket*                             socket;
};
