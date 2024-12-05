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

// debug messasges (print request and response ???)

// fds:
// -    if exit (need to close all open fds?)
// -    check leaks

// config parser:
// -    check cgi path (execute rights) and if cgi to the extension exists

// HttpRequestParser:
// -    parse the chunk extensions and trailer section or keep ignoring ???
// -    do Percent decoding ???

// Response
// -    send gif -> loads too long
// -    check for one and a valid host header(if not -> 400 bad request) read rfc for more detail
// -    do we sent the response in packets ? or chuncked or at once

