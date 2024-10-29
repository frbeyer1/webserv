#include "../inc/Webserv.hpp"

int main(int argc, char **argv)
{
    std::string config;

    if (argc == 1)
        config = DEFAULT_CONFIG;
    else if (argc == 2)
        config = argv[1];
    else
    {
        Logger::log(RED, ERROR, std::ostringstream() << "Invalid Arguments: try './webserv [configuration file]'");
        return (EXIT_FAILURE);
    }
    ServerManager   master;

    master.configure(config);
    // master.setup();
    // master.boot();
    return (EXIT_SUCCESS);
}

// ==============   TO DO   ================== //

// HttpRequestParser:
// -    parse the chunk extensions and trailer section or keep ignoring ???
// -    do Percent decoding ???
// -    check for one and a valid host header(if not -> 400 bad request) read rfc for more detail

// fds:
// -    if exit (need to close all fds?)
// -    server sockets also non blocking???

// config parser:
// -    what if location directive is empty
// -    should directives with paths have the full path?? or dependent on root (so it needs root first???)
//      -> takes root if there , and takes full path when no root
// -    max config lenght???

// other stuff:
// -    init default error pages

// use read or recv and write or send ???
