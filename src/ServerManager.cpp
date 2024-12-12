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
static void    addToEpollInstance(int epoll_fd, int add_fd)
{
    struct epoll_event event;
    event.events = EPOLLIN; 
    event.data.fd = add_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, add_fd, &event) == -1)
    {
        Logger::log(RED, ERROR, "adding fd[%i] to epoll instance failed", add_fd);
        exit(EXIT_FAILURE);
    }
}
/*
adds the add_fd to the epoll instance
*/
static void    addServerToEpollInstance(int epoll_fd, int add_fd)
{
    struct epoll_event event;
    event.events = EPOLLIN; 
    event.data.fd = add_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, add_fd, &event) == -1)
    {
        Logger::log(RED, ERROR, "adding fd[%i] to epoll instance failed", add_fd);
        exit(EXIT_FAILURE);
    }
}

// ======   Private member functions   ======= //
/*
accepting a new connection:
    - adding the new client into the client_map 
    - adding client_fd to the epoll instance
*/
void    ServerManager::_acceptNewConnection(int socket_fd)
{
    if (_client_map.size() >= MAX_CONNECTIONS)
    {
        Logger::log(YELLOW, INFO, "Did not accept new connection, because there are allready MAX_CONNECTIONS[%i]", MAX_CONNECTIONS);
        return ;
    }

    // initializing the Client struct
    Client  client;

    client.socket = &_socket_map[socket_fd];
    client._client_fd = client.socket->acceptConnection();
    client.request.setSocket(client.socket);
    client.request.setServerBlocks(_server_blocks);
    client._client_address = client.socket->getSocketAddress();
    if (_client_map.count(client._client_fd) > 0)
        _client_map.erase(client._client_fd);
    client._last_msg_time = time(NULL);
    _client_map.insert(std::make_pair(client._client_fd, client));
    addToEpollInstance(_epoll_fd, client._client_fd);
    Logger::log(CYAN, INFO, "Accpted new connection on fd[%i] from address[%s]", client._client_fd, inAddrToIpString(client._client_address.sin_addr.s_addr).c_str());
}

/*
closes connection:
    - removing the client_fd fromt the epoll instance
    - closing the client_fd
    - removing the client from the client_map
*/
void    ServerManager::_closeConnection(int fd)
{
    if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1)
    {
        Logger::log(RED, ERROR, "Deleting fd[%i] from epoll instance failed", fd);
        exit(EXIT_FAILURE);
    }
    if (close(fd))
    {
        Logger::log(RED, ERROR, "Closing fd[%i] failed", fd);
        exit(EXIT_FAILURE);
    }
    _client_map.erase(fd);
    Logger::log(CYAN, INFO, "Closed connection on fd[%i]", fd);
}

/*
checking for timeouts of clients in the _client_map
*/
void    ServerManager::_checkTimeout()
{
    std::vector<int> timeouts;

    for (std::map<int, Client>::iterator it = _client_map.begin(); it != _client_map.end(); it++)
    {
        if (time(NULL) - it->second._last_msg_time > CLIENT_CONNECTION_TIMEOUT)
            timeouts.push_back(it->first);
    }
    for (size_t i = 0; i < timeouts.size(); i++)
    {
        Logger::log(YELLOW, INFO, "Client timeout: Client_FD[%i], closing connection ...", timeouts[i]);
        _closeConnection(timeouts[i]);
    }
}

/*
Reading of the HTTP Request:
    - reading READ_SIZE amount of octest from the client into an buffer
    - parsing the buffer into an HttpRequest object
    - set epoll settings to EPOLLOUT on client_fd if recieved full request
*/
void    ServerManager::_readRequest(Client &client)
{
    uint8_t buffer[REQUEST_READ_SIZE];
    int     bytes_read = 0;
    int     fd = client._client_fd;

    bytes_read = read(fd, buffer, REQUEST_READ_SIZE);
    if (bytes_read == 0)
    {
        Logger::log(CYAN, INFO, "Client fd[%i] closed connection", fd);
        _closeConnection(fd);
        return ;
    }
    if (bytes_read < 0)
    {
        Logger::log(RED, ERROR, "Read error on fd[%i]", fd);
        _closeConnection(fd);
        return ;
    }
    else
    {
        client._last_msg_time = time(NULL);
        client.request.parse(buffer, bytes_read);
        std::memset(buffer, 0, sizeof(buffer));
    }
    if (client.request.getParsingState() == Parsing_Finished || client.request.getError() != OK)
    {
        Logger::log(CYAN, INFO, "Request received from client fd[%i] with method[%s] and URI[%s]", fd, client.request.getMethodStr().c_str(), client.request.getPath().c_str());
        client.response.buildResponse(client.request);
        Logger::log(GREY, DEBUG, "Finished response building");
        struct epoll_event event;

        event.events = EPOLLOUT;
        event.data.fd = fd;
        if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, fd, &event))
        {
            Logger::log(RED, ERROR, "Changing settings associated with fd[%i] in epoll instance failed", fd);
            exit(EXIT_FAILURE);
        }
    }
}

/*
sending the Response to the client:
    - write RESPONSE_WRITE_SIZE to client_Fd
    - clear reuquest and response objects after sending full response
    - set epoll settings to EPOLLIN after sending response to client
    - check if connection should be "keep-alive"
*/
void    ServerManager::_sendResponse(Client &client)
{
    int bytes_send = 0;
    int fd = client._client_fd;

    const std::string &response = client.response.getResponse();

    if (response.size() >= RESPONSE_WRITE_SIZE)
        bytes_send = write(fd, response.c_str(), RESPONSE_WRITE_SIZE);
    else
        bytes_send = write(fd, response.c_str(), response.size());
    if (bytes_send < 0)
    {
        Logger::log(RED, ERROR, "Write error on fd[%i]: client closed Connection", fd);
        _closeConnection(fd);
        return ;
    }
    if (bytes_send == 0 || (size_t)bytes_send == response.size())
    {
        Logger::log(CYAN, INFO, "Response send to client fd[%i] with code[%i]", client._client_fd, client.response.getError());
        if (client.response.getConnection() == "keep-alive")
        {
            struct epoll_event event;

            event.events = EPOLLIN;
            event.data.fd = fd;
            if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, fd, &event))
            {
                Logger::log(RED, ERROR, "Changing settings associated with fd[%i] in epoll instance failed", _epoll_fd);
                exit(EXIT_FAILURE);
            }
            client.response.clear();
            client.request.clear();
        }
        else
            _closeConnection(fd);
    }
    else
        client.response.trimResponse(bytes_send);
}

// ==========   Member functions   =========== //
/*
setting up all server
    - parsing the config file
    - setting up all sockets and adding them to the _socket_map
*/
void    ServerManager::setup(std::string config)
{
    Logger::log(WHITE, INFO, "Setting up Servers ...");

    // parsing of the config file
    ConfigParser        parser(_server_blocks);

    parser.parse(config);
    Logger::log(GREY, DEBUG, "Finished config file parsing");
    if (_server_blocks.size() == 0)
    {
        Logger::log(RED, ERROR, "Config File: no server block found ( empty file ? )");
        exit(EXIT_FAILURE);
    }

    // Printing server setup
    for (size_t i = 0; i < _server_blocks.size(); i++)
    {
        std::string server_name = "";
        if (!_server_blocks[i]._server_names.empty())
            server_name = _server_blocks[i]._server_names[0];
        Logger::log(WHITE, INFO, "Server setup: Name[%s] Host[%s] Port[%i]", server_name.c_str(), _server_blocks[i]._ip.c_str(), _server_blocks[i]._port);
    }

    // find all needed sockets
    std::map<uint16_t, in_addr_t> map;

    for (size_t i = 0; i < _server_blocks.size(); i++)
        map.insert(std::pair<uint16_t, in_addr_t>(_server_blocks[i]._port, _server_blocks[i]._host));

    // set host and port for all needed sockets
    std::vector<Socket> sockets;

    for (std::map<uint16_t, in_addr_t>::iterator it = map.begin(); it != map.end(); it++) 
    {
        Socket socket;

        socket.setPort(it->first);
        socket.setHost(it->second);
        sockets.push_back(socket);
    }

    // setup all sockets and add to _socket_map
    Logger::log(GREY, DEBUG, "Setting up sockets ...");
    for (size_t i = 0; i < sockets.size(); i++)
    {
        sockets[i].setup();
        _socket_map.insert(std::pair<int, Socket>(sockets[i].getSocketFd(), sockets[i]));
        Logger::log(GREY, DEBUG, "Socket setup: Host[%s] Port[%i]", inAddrToIpString(htonl(sockets[i].getHost())).c_str(), sockets[i].getPort());
    }

    // assign sockets to servers
    for (size_t i = 0; i < _server_blocks.size(); i++)
    {
        for (std::map<int, Socket>::iterator it = _socket_map.begin(); it != _socket_map.end(); it++)
        {
            if (it->second.getHost() == _server_blocks[i]._host && it->second.getPort() == _server_blocks[i]._port)
            {
                _server_blocks[i]._socket = &it->second;
                break ;
            }
        }
    }
    Logger::log(GREY, DEBUG, "Setting up Sockets finished");
}

/*
booting the servers:
    - creating the epoll instance
    - adding server_fds to the epoll instance
    - start listening on the server sockets
main server loop:
    - waiting for events on the fds of the epoll instance
    - handling the epoll event list
    - checking for timeouts
*/
void    ServerManager::boot()
{
    Logger::log(WHITE, INFO, "Booting Servers ...");

    // creates epoll instance
    _epoll_fd = epoll_create(1);
    if (_epoll_fd == -1)
    {
        Logger::log(RED, ERROR, "Creating epoll instace failed");
        exit(EXIT_FAILURE);
    }

    // adds all server_fds to epoll instance and starts listening on the server sockets
    for (std::map<int, Socket>::iterator it = _socket_map.begin(); it != _socket_map.end(); it++)
    {
        addServerToEpollInstance(_epoll_fd, it->second.getSocketFd());
        it->second.startListening();
    }
    Logger::log(WHITE, INFO, "Booted Servers successfully");
    // main server loop
    struct epoll_event event_list[MAX_EPOLL_EVENTS];

    while (true)
    {
        // wating for events on the epoll instance
        int num_events = epoll_wait(_epoll_fd, event_list, MAX_EPOLL_EVENTS, -1);
        if (num_events == -1)
        {
            Logger::log(RED, ERROR, "Waiting for event on the epoll instance failed");
            exit(EXIT_FAILURE);
        }
        // handling of the epoll event list
        for (int i = 0; i < num_events; i++)
        {
            int fd = event_list[i].data.fd;

            if (_socket_map.count(fd))
                _acceptNewConnection(fd);
            else if (_client_map.count(fd) && event_list[i].events & EPOLLIN)
                _readRequest(_client_map[fd]);
            else if (_client_map.count(fd) && event_list[i].events & EPOLLOUT)
                _sendResponse(_client_map[fd]);
            else
                close(fd);
        }
        _checkTimeout();
    }
}
