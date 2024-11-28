#include "../inc/Webserv.hpp"
#include "../inc/ServerManager.hpp"

int main(int argc, char **argv)
{
    std::string config;

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

    master.setup(config);
    master.boot();
}

// ==============   TO DO   ================== //

// use read or recv and write or send ???

// change Server on client to a refertenct to a server 

// fds:
// -    if exit (need to close all fds?)

// config parser:
// -    delete index in server block
// -    handle alias and cgi
// -    what if location directive is empty
// -    max config lenght???

// HttpRequestParser:
// -    parse the chunk extensions and trailer section or keep ignoring ???
// -    do Percent decoding ???

// Response
// -    check for one and a valid host header(if not -> 400 bad request) read rfc for more detail
