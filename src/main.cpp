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
    //master.setup();
    //master.boot();
    return (EXIT_SUCCESS);
}

// ==============   TO DO   ================== //

// HttpRequestParser:
//  -    parse the chunk extensions and trailer section or keep ignoring ???
//  -    do Percent decoding ???

// if exit (need to close all fds?)

// server sockets also non blocking???

// max config lenght???

// init server_map

// init default error pages

// request -> check for one and valid host header(if not -> 400 bad request)

// logger info msgs for accepting closing connections

// error page directive parsing: check for existing error pages -> replace them

// check for read rights for error page