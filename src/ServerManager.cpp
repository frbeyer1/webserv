#include "../inc/ServerManager.hpp"

void    ServerManager::configure(std::string config)
{
    ConfigParser    parser(_servers);

    parser.parse(config);
    

}

void    ServerManager::setup()
{
    bool    duplicate;

    for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); it++)
    {
        duplicate = false;
        for (std::vector<Server>::iterator it2 = _servers.begin(); it != _servers.end(); it2++)
        {
            if (it->getHost() == it2->getHost() && it->getPort() == it2->getPort())
                duplicate = true;
        }
        if (!duplicate)
            it->setup();
    }
}


void    ServerManager::boot()
{

}