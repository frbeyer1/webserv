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
        Logger::log(RED, ERROR, "Invalid Arguments: try './webserv [configuration file]'");
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

// ======= TO DO ========

// RESPONSE:
// - check that if the uploaded file is allready there it updates it, and returns 200 OK or 204 (if 200 how to print success page)
// - POST with an script.py what is not there -> should upload or should say not found ???
// - multi file (multipart/form-data) and plain/text upload possible ??? (and what happens when content-type is something else)
// - if there is an cgi script, but this kind of cgi is not in the config -> gets the script as normal content is this right ????

// CGI:
// - set REMOTE_ADDR & REMOTE_HOST & REMOTE_IDENT & REMOTE_USER
// - make an cgi script to test POST request with cgi
// - check for infinite loop in cgi -> kill cgi after while
