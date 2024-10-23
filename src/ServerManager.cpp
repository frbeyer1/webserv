#include "../inc/ServerManager.hpp"

// =============   Constructor   ============= //
ServerManager::ServerManager()
{
}

// ============   Deconstructor   ============ //
ServerManager::~ServerManager()
{
}

// ================   Utils   ================ //
/*
adds the add_fd to the epoll instance
*/
static void addToEpollInstance(int epoll_fd, int add_fd)
{
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLOUT; 
    event.data.fd = add_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, add_fd, &event) == -1)
    {
        Logger::log(RED, ERROR, "adding to epoll instance failed");
        exit(EXIT_FAILURE);
    }
}

/*
Checks for duplicates in the server list (same Host and Port)
*/
static  bool    checkDuplicates(std::vector<Server> &servers)
{
    bool    duplicate;

    duplicate = false;
    for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); it++)
    {
        for (std::vector<Server>::iterator it2 = servers.begin(); it != servers.end(); it2++)
        {
            if (it->getHost() == it2->getHost() && it->getPort() == it2->getPort())
                duplicate = true;
        }
    }
    return (duplicate);
}

// ==========   Member functions   =========== //
/*
parses the config file
*/
void    ServerManager::configure(std::string config)
{
    ConfigParser    parser(_servers);

    parser.parse(config);
    Logger::log(WHITE, DEBUG, "Finished config file parsing");
}

/*
setting up all servers
*/
void    ServerManager::setup()
{
    Logger::log(WHITE, INFO, "Setting up servers...");
    if (checkDuplicates(_servers))
    {
        Logger::log(RED, ERROR, "Could not setup servers, because of duplicates in config file");
        exit(EXIT_FAILURE);
    }
    for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); it++)
    {
        it->setup();
        _server_map.insert(std::pair<int, Server>(it->getServerFd(), *it));
        std::string msg;
        msg = "Server setup: ServerName[" + it->getSeverName() + "] Host[" + it->getIp() + "] Port[";
        msg += it->getPort() + "]";
        Logger::log(WHITE, INFO, msg.c_str());
    }
    Logger::log(WHITE, DEBUG, "Setting up servers finished");
}

/*
accepts new connection and puts the new client into the client_map and epoll instance
*/
void    ServerManager::acceptNewConnection(int server_fd)
{
    int new_socket;

    if (_server_map.size() > MAX_CONNECTIONS)
    {
        Logger::log(YELLOW, INFO, "Did not accept connection, because there are more than MAX_CONNECTIONS");
        return ;
    }
    new_socket = _server_map[server_fd].acceptConnection();

    Client client;

    client.setClientFd(new_socket);
    _client_map.insert(std::pair<int, Client>(new_socket, client));
    addToEpollInstance(_epoll_fd, new_socket);
}

/*
closes connection and removes the client from the client_map and epoll instance
*/
void    ServerManager::closeConnection(int fd)
{
    if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1)
    {
        Logger::log(RED, ERROR, "deleting from epoll instance failed");
        exit(EXIT_FAILURE);
    }
    close(fd);
    _client_map.erase(fd);
    std::string msg;
    msg = "closed connection " + fd;
    Logger::log(WHITE, DEBUG, "closed Connection");
}

/*
checks for timeouts on all clients
*/
void    ServerManager::checkTimeout()
{
    for (std::map<int, Client>::iterator it = _client_map.begin(); it != _client_map.end(); it++)
    {
        if (time(NULL) - it->second.getLastMsgTime() > CLIENT_CONNECTION_TIMEOUT)
        {
            std::string msg;
            msg = "Client timeout: Client_FD[" + it->second.getClientFd();
            msg += "], closing connection ...";
            Logger::log(WHITE, INFO, msg.c_str());
            closeConnection(it->first);
        }
    }
}

/*
starts listening on the servers and setting up of the epoll instance for I/O multiplexing
*/
void    ServerManager::boot()
{
    Logger::log(WHITE, INFO, "Booting Servers ...");
    // creates epoll instance
    int _epoll_fd = epoll_create(1);
    if (_epoll_fd == -1)
    {
        Logger::log(RED, ERROR, "creating epoll instace failed");
        exit(EXIT_FAILURE);
    }
    // adds all server_fds to epoll instance and starts listening on them
    for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); it++)
    {
        addToEpollInstance(_epoll_fd, it->getServerFd());
        it->startListening();
    }
    struct epoll_event event_list[MAX_EPOLL_EVENTS];
    while (true)
    {
        int num_events = epoll_wait(_epoll_fd, event_list, MAX_EPOLL_EVENTS, -1);
        if (num_events == -1)
        {
            Logger::log(RED, ERROR, "waiting for event on the epoll instance failed");
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < num_events; i++)
        {
            int fd = event_list[i].data.fd;
            
            if (_server_map.count(fd))
                acceptNewConnection(fd);
            // else if (_client_map.count(fd) && event_list[i].events & EPOLLIN)
                // readRequest();
            // else if (_client_map.count(fd) && event_list[i].events & EPOLLOUT)
                // sendResponse();
            // else
                // close(fd);
        }
        checkTimeout();
    }
}
