#include "../inc/Webserv.hpp"

int main(int argc, char **argv)
{
    std::string config;

    Logger::setState(LOGGER_STATE);
    Logger::setOutputMode(LOGGER_OUTPUT_MODE);
    Logger::setLogLvl(LOGGER_LOG_LVL);
    if (argc == 1)
        config = DEFAULT_CONFIG;
    else if (argc == 2)
        config = argv[1];
    else
    {
        Logger::log(RED, ERROR, "Invalid Arguments: try './webserv [configuration file]'");
        return (EXIT_FAILURE);
    }
    ServerManager   master;

    master.configure(config);
    master.setup();
    master.boot();
    return (EXIT_SUCCESS);
}

// if exit (need to close all fds?)

// make fds non blocking
// server sockets also non blocking???

// give max_body_size to RequestClass

// max connections???

// close connection after response or use timeouts

// max config lenght???