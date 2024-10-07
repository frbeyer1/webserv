#include "../inc/Webserv.hpp"

int main(int argc, char **argv)
{
    std::string config;

    if (argc == 1)
        config = "conf/default.conf";
    else if (argc == 2)
        config = argv[1];
    else
    {
        // error msg
        return (EXIT_FAILURE);
    }
    ServerManager   master;

    master.configure(config);
    master.setup();
    master.boot();
    return (EXIT_SUCCESS);
}

// to do 
// -logger
// if exit (need to close all fds?)
// start listening for all servers
// make fds non blocking
// setup server map
// give max_body_size to RequestClass
// structuring the class with comments