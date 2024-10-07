#pragma once

#include "Webserv.hpp"

class ServerManager
{
private:
    std::vector<Server>       _servers;

public:

    void    configure(std::string config);
    void    setup();
    void    boot();

};
