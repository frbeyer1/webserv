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
    std::vector<Server>&    _servers;
    std::string             _content;
    int                     _i;

// private member functions
    void        _readConfig(std::string config);
    void        _findServerBlock();
    void        _skipWhiteSpaces();
    void        _skipComment();
    void        _getDirective(Server &server);
    Directive   _getDirectiveType();
    std::string _getParameter();
    void        _getLocation(Server &server);

public:
// Constructor
    ConfigParser(std::vector<Server> &servers);

// Deconstructor
    ~ConfigParser();

// Member functions
    void    parse(std::string config);

};

// Utils
uint32_t ipStringToNumeric(const std::string& ip);