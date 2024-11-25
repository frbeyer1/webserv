#pragma once

#include "Webserv.hpp"

class Response
{
private:
    std::string   _response_str;

// Private member functions
    std::string _buildDefaultErrorPage(int error_code);

public:
// Constructor
    Response();

// Deconstructor
    ~Response();

// Member functions
    void    build(HttpRequest &request);

// Getter
    std::string &getResponseStr();

};
