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
    for (size_t i = 0; i < servers.size(); ++i)
    {
        for (size_t j = i + 1; j < servers.size(); ++j)
        {
            if (servers[i].getHost() == servers[j].getHost() && servers[i].getPort() == servers[j].getPort())
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
        std::ostringstream oss;
        oss << "Did not accept connection, because there are more than " << MAX_CONNECTIONS;
        Logger::log(YELLOW, INFO, oss.str());
        return ;
    }
    int     client_fd;
    Client  client(_server_map[server_fd]);

    client_fd = _server_map[server_fd].acceptConnection();
    client.setClientFd(client_fd);
    client.setClientAddress(_server_map[server_fd].getSocketAddress());
    if (_client_map.count(client_fd) > 0)
        _client_map.erase(client_fd);
    _client_map.insert(std::make_pair(client_fd, client));
    addToEpollInstance(_epoll_fd, client_fd);
    std::ostringstream oss;
    oss << "Accpted new Connection from " << sockaddrToIpString(client.getClientAddress()) << ", on fd " << client_fd;
    Logger::log(WHITE, INFO, oss.str());
}

/*
closes connection and removes the client from the client_map and epoll instance
*/
void    ServerManager::_closeConnection(int fd)
{
    if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1)
    {
        Logger::log(RED, ERROR, "deleting from epoll instance failed");
        exit(EXIT_FAILURE);
    }
    close(fd);
    _client_map.erase(fd);
    std::ostringstream oss;
    oss  << "Closed Connection " << fd;
    Logger::log(WHITE, INFO, oss.str());
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
            std::ostringstream oss;
            oss  << "Client timeout: Client_FD[" << it->second.getClientFd() << "], closing connection ...";
            Logger::log(WHITE, INFO, oss.str());
            _closeConnection(it->first);
        }
    }
}

/*
reads READ_SIZE amount of octets of the fd and feeds it to the HttpRequest parser
*/
void    ServerManager::_readRequest(int fd, Client &client)
{
    uint8_t buffer[READ_SIZE];
    int     bytes_read;

    bytes_read = read(fd, buffer, READ_SIZE);
    if (bytes_read < 0)
    {
        std::ostringstream oss;
        oss << "Error: read error on fd " << fd;
        Logger::log(RED, ERROR, oss.str());
    }
    if (bytes_read <= 0)
    {
        _closeConnection(fd);
        return ;
    }
    else
    {
        client.setLastMsgTime(time(NULL));
        client.request.parse(buffer, bytes_read);
        std::memset(buffer, 0, sizeof(buffer));
            std::cout <<  "readrequest" << std::endl;
        if (client.request.getParsingState() == Parsing_Finished)
        {
            client.response.buildResponse(client.request);
            client.request.clear();
        }
    }
}

/*
sends the already build response
*/
void    ServerManager::_sendResponse(int fd, Client &client)
{
    std::string &respons_str = client.response.getResponseStr();
    // getTargetFile();
    std::cout << respons_str << std::endl;
    write(fd, respons_str.c_str(), respons_str.size());
    exit(1);
}

// ==========   Member functions   =========== //
/*
parses the config file
*/
void    ServerManager::configure(std::string config)
{
    ConfigParser    parser(_servers);

    parser.parse(config);
    Logger::log(GREY, DEBUG, "Finished config file parsing");
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
        std::ostringstream oss;
        oss << "Server setup: ServerName[" << it->getSeverName() << "] Host[" << it->getIp() << "] Port[" << it->getPort() << "]";
        Logger::log(WHITE, INFO, oss.str());
    }
    Logger::log(GREY, DEBUG, "Setting up servers finished");
}

/*
starts listening on the servers and setting up of the epoll instance for I/O multiplexing
*/
void    ServerManager::boot()
{
    Logger::log(WHITE, INFO, "Booting Servers ...");
    // creates epoll instance
    _epoll_fd = epoll_create(1);
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
                _acceptNewConnection(fd);
            else if (_client_map.count(fd) && event_list[i].events & EPOLLIN)
                _readRequest(fd, _client_map[fd]);
            else if (_client_map.count(fd) && event_list[i].events & EPOLLOUT)
                _sendResponse(fd, _client_map[fd]);
            else
                close(fd);
        }
        _checkTimeout();
    }
}
