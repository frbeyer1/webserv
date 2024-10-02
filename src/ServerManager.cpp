#include "../inc/ServerManager.hpp"

void    ServerManager::configure(std::string config)
{
    std::ifstream   file(config);

    if (file.fail())
    {
        std::cerr << "Error: Unable to open file " << config << std::endl;
        exit(EXIT_FAILURE);
    }
    std::stringstream buffer;
    
    buffer << file.rdbuf();

    std::string content = buffer.str();
    
    // parsing the config file into std::vector<Server> _server;

    file.close();
}

