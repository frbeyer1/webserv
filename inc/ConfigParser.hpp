#pragma once

#include "Webserv.hpp"

class ServerBlock;

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
    size_t                      _i;
    std::string                 _content;
    std::vector<ServerBlock>&   _server_blocks;

// Private Member functions
    void        _readConfig(std::string config);
    void        _skipComment();
    void        _skipWhiteSpaces();
    void        _findNextServerBlock();
    std::string _getParameter();
    Directive   _getDirectiveType();
    std::string _getLocationPath();
    void        _getLocation(ServerBlock &server_block);
    void        _getDirective(ServerBlock &server_block);
    void        _setDefaultValues(ServerBlock &server_block);

public:
// Constructor
    ConfigParser(std::vector<ServerBlock> &server_blocks);

// Deconstructor
    ~ConfigParser();

// Member functions
    void    parse(std::string config);

};

// Utils
uint32_t    ipStringToNumeric(const std::string& ip);
std::string inAddrToIpString(in_addr_t addr);
