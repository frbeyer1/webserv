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

// max connections???

// close connection after response (handles only full requests in one sending) (chunked transefer would make no sense) or use timeouts

// max config lenght???

// how to config max body size ??? in MB or in bytes

// init server_map

// add logger info and debug msgs

// init default error pages