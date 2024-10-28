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
        Logger::log(RED, ERROR, std::ostringstream() << "adding to epoll instance failed");
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

// ======   Private member functions   ======= //
/*
accepts new connection and puts the new client into the client_map and epoll instance
*/
void    ServerManager::_acceptNewConnection(int server_fd)
{
    if (_server_map.size() > MAX_CONNECTIONS)
    {
        Logger::log(YELLOW, INFO, std::ostringstream() << "Did not accept connection, because there are more than " << MAX_CONNECTIONS);
        return ;
    }
    int     client_fd;
    Client  client(_server_map[server_fd]);

    client_fd = _server_map[server_fd].acceptConnection();
    client.setClientFd(client_fd);
    client.setClientAddress(_server_map[server_fd].getSocketAddress());
    addToEpollInstance(_epoll_fd, client_fd);
    if (_client_map.count(client_fd) != 0)
        _client_map.erase(client_fd);
    _client_map.insert(std::make_pair(client_fd, client));
    Logger::log(WHITE, INFO, std::ostringstream() << "Accpted new Connection from " << client.getClientAddress().sin_addr.s_addr << ", on fd " << client_fd);
}

/*
closes connection and removes the client from the client_map and epoll instance
*/
void    ServerManager::_closeConnection(int fd)
{
    if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1)
    {
        Logger::log(RED, ERROR, std::ostringstream() << "deleting from epoll instance failed");
        exit(EXIT_FAILURE);
    }
    close(fd);
    _client_map.erase(fd);
    Logger::log(WHITE, INFO, std::ostringstream() << "closed Connection " << fd);
}

/*
checks for timeouts on all clients
*/
void    ServerManager::_checkTimeout()
{
    for (std::map<int, Client>::iterator it = _client_map.begin(); it != _client_map.end(); it++)
    {
        if (time(NULL) - it->second.getLastMsgTime() > CLIENT_CONNECTION_TIMEOUT)
        {
            Logger::log(WHITE, INFO, std::ostringstream() << "Client timeout: Client_FD[" << it->second.getClientFd() << "], closing connection ...");
            _closeConnection(it->first);
        }
    }
}

/*
reads READ_SIZE amount of octets of the fd and feeds it to the HttpRequest parser
*/
void    ServerManager::_readRequest(int fd)
{
    uint8_t buffer[READ_SIZE];
    int     bytes_read;

    bytes_read = read(fd, buffer, READ_SIZE);
    if (bytes_read == 0)
    {
        _closeConnection(fd);
        return ;
    }
    else if (bytes_read < 0)
    {
        Logger::log(RED, ERROR, std::ostringstream() << "Error: read error on fd " << fd);
        _closeConnection(fd);
        return ;
    }
    else
    {
        Client &client = _client_map[fd];
        client.request.parse(buffer, bytes_read);
        client.setLastMsgTime(time(NULL));
        memset(buffer, 0, sizeof(buffer));
    }

    // check for parsing finished -> build response
    // clear http request object
}

/*

*/
void    ServerManager::_sendResponse(int fd)
{
    // send response
}

// ==========   Member functions   =========== //
/*
parses the config file
*/
void    ServerManager::configure(std::string config)
{
    ConfigParser    parser(_servers);

    parser.parse(config);
    Logger::log(WHITE, DEBUG, std::ostringstream() << "Finished config file parsing");
}

/*
setting up all servers
*/
void    ServerManager::setup()
{
    Logger::log(WHITE, INFO, std::ostringstream() << "Setting up servers...");
    if (checkDuplicates(_servers))
    {
        Logger::log(RED, ERROR, std::ostringstream() << "Could not setup servers, because of duplicates in config file");
        exit(EXIT_FAILURE);
    }
    for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); it++)
    {
        it->setup();
        _server_map.insert(std::pair<int, Server>(it->getServerFd(), *it));
        Logger::log(WHITE, INFO, std::ostringstream() << "Server setup: ServerName[" << it->getSeverName() << "] Host[" << it->getIp() << "] Port[" << it->getPort() << "]");
    }
    Logger::log(WHITE, DEBUG, std::ostringstream() << "Setting up servers finished");
}

/*
starts listening on the servers and setting up of the epoll instance for I/O multiplexing
*/
void    ServerManager::boot()
{
    Logger::log(WHITE, INFO, std::ostringstream() << "Booting Servers ...");
    // creates epoll instance
    int _epoll_fd = epoll_create(1);
    if (_epoll_fd == -1)
    {
        Logger::log(RED, ERROR, std::ostringstream() << "creating epoll instace failed");
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
            Logger::log(RED, ERROR, std::ostringstream() << "waiting for event on the epoll instance failed");
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < num_events; i++)
        {
            int fd = event_list[i].data.fd;
            
            if (_server_map.count(fd))
                _acceptNewConnection(fd);
            else if (_client_map.count(fd) && event_list[i].events & EPOLLIN)
                _readRequest(fd);
            else if (_client_map.count(fd) && event_list[i].events & EPOLLOUT)
                _sendResponse(fd);
            else
                close(fd);
        }
        _checkTimeout();
    }
}
