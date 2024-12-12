#pragma once

#include "Webserv.hpp"

class Socket;

struct AllowedMethods
{
    bool                                _allow_get;
    bool                                _allow_delete;
    bool                                _allow_post;
};

struct Location
{
    std::string                         _alias;
    std::string                         _index;
    std::string                         _upload;
    std::string                         _redirection;
    AllowedMethods                      _allowed_methods;
    std::map<std::string, std::string>  _cgi;
    bool                                _autoindex;
};

struct ServerBlock
{
    std::vector<std::string>            _server_names;
    in_addr_t                           _host;
    uint16_t                            _port;
    std::string                         _ip;
    std::string                         _root;
    size_t                              _client_max_body_size;
    std::map<int, std::string>          _error_pages;
    std::map<std::string, Location>     _locations;
    Socket*                             _socket;
};
