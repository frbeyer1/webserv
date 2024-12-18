#include "../inc/Webserv.hpp"
#include "../inc/ServerManager.hpp"

void sigpipe_handler(int signum) 
{
    (void)signum;
}

int main(int argc, char ** argv)
{
    signal(SIGPIPE, sigpipe_handler);
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

// fds:
// -    if exit (need to close all open fds?)
// -    check leaks

// check all exit() and delete if in the main server loop

// response: readStatus: still hardcoded => do nothing after post or send succsess Page ?
