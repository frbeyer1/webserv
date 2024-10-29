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
    UPLOAD,
    CGI,
    LOCATION,
    UNKNOWN,
};

class ConfigParser
{
private:
    std::vector<Server>     &_server_vector;
    std::string             _content;
    size_t                  _i;

// Private Member functions
    void        _readConfig(std::string config);
    void        _skipComment();
    void        _skipWhiteSpaces();
    void        _findNextServerBlock();
    std::string _getParameter();
    Directive   _getDirectiveType();
    std::string _getLocationPath();
    void        _getLocation(Server &server);
    void        _getDirective(Server &server);


public:
// Constructor
    ConfigParser(std::vector<Server> &server_vector);

// Deconstructor
    ~ConfigParser();

// Member functions
    void    parse(std::string config);

};

// Utils
uint32_t ipStringToNumeric(const std::string& ip);
