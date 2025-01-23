#include "../inc/Webserv.hpp"
#include "../inc/ServerManager.hpp"

void signal_handler(int signal)
{ 
    (void)signal;
}

int main(int argc, char **argv)
{
    std::string config;

    if (argc == 1)
        config = DEFAULT_CONFIG;
    else if (argc == 2)
        config = argv[1];
    else
    {
        Logger::log(RED, ERROR, "Invalid Arguments: try './webserv <path/to/config>'");
        return (EXIT_FAILURE);
    }
    if (signal(SIGPIPE, signal_handler) == SIG_ERR)
    {
        Logger::log(RED, ERROR, "Failed to ignore SIGPIPE");
        return (EXIT_FAILURE);
    }
    ServerManager   master;

    master.setup(config);
    master.boot();
}
