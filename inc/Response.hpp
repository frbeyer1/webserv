#pragma once

#include "Webserv.hpp"

class Response
{
private:

// Private member functions
    std::string _buildDefaultErrorPage(int error_code);

public:
// Constructor
    Response();

// Deconstructor
    ~Response();


};