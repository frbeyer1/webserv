#pragma once

#include "Webserv.hpp"

class Response
{
private:
    // std::stringstream   _response_str;
    // size_t              _response_size;

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
    // std::string getResponseStr();
    // size_t   getResponseSize();

};
