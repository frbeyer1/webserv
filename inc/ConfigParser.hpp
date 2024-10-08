#pragma once

#include "Webserv.hpp"

enum Directive
{
    ROOT,
    LISTEN,
    SERVER_NAME,
    CLIENT_MAX_BODY_SIZE,
    ERROR_PAGE,
    ALLOWED_METHODS,
    REDIRECTION,
    ALIAS,
    AUTOINDEX,
    INDEX,
    // ...
    LOCATION,
    UNKNOWN,
};

class ConfigParser
{
private:
    std::vector<Server> &_servers;

public:
// Constructor
    ConfigParser(std::vector<Server> &servers);

// Deconstructor
    ~ConfigParser();

// Member functions
    void    parse(std::string config);

};
