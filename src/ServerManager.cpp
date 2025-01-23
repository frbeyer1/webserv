#include "../inc/ServerManager.hpp"

// =============   Constructor   ============= //

ServerManager::ServerManager()
{
    _epoll_fd = 0;
}

// ============   Deconstructor   ============ //

ServerManager::~ServerManager()
{
}

// ================   Utils   ================ //

/*
adds the add_fd to the epoll instance for EPOLLIN events
Returns zero on success, -1 on error
*/
static int    addListeningSocketToEpoll(int epoll_fd, int add_fd, int client_fd)
{
    struct epoll_event listening_event;

	e_data* data = new e_data;
	data->fd = add_fd;
	data->client_fd = client_fd;
	listening_event.data.ptr = data;
	listening_event.events = EPOLLIN;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, add_fd, &listening_event) == -1)
        return -1;
    return 0;
}

static e_data*    addToEpollInstanceEpollin(int epoll_fd, int add_fd, int client_fd)
{
    struct epoll_event listening_event;

	e_data* data = new e_data;
	data->fd = add_fd;
	data->client_fd = client_fd;
	listening_event.data.ptr = data;
	listening_event.events = EPOLLIN;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, add_fd, &listening_event) == -1)
        return NULL;
    return data;
}

static e_data*    addToEpollInstanceEpollout(int epoll_fd, int add_fd, int client_fd)
{
    struct epoll_event listening_event;

	e_data* data = new e_data;
	data->fd = add_fd;
	data->client_fd = client_fd;
	listening_event.data.ptr = data;
	listening_event.events = EPOLLOUT;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, add_fd, &listening_event) == -1)
        return NULL;
    return data;
}

// ======   Private member functions   ======= //

/*
accepting a new connection:
    - checking for MAX_CONNECTIONS
    - initializing the Client struct
    - accepting the new connection on the socket
    - adding client_fd to the epoll instance
    - adding the new client into the client_map 
*/
void    ServerManager::_acceptNewConnection(int socket_fd)
{
    // checking for MAX_CONNECTIONS
    if (_client_map.size() >= MAX_CONNECTIONS)
    {
        Logger::log(YELLOW, INFO, "Did not accept new connection, because there are allready MAX_CONNECTIONS[%i]", MAX_CONNECTIONS);
        return ;
    }

    // searching socket_fd in the _socket_map
    if (_socket_map.count(socket_fd) == 0)
    {
        Logger::log(RED, ERROR, "Could not find Socket in the socket_map");
        return ;
    }

    // accept connection on socket
    Client client;

    client.socket = &_socket_map[socket_fd];
    if ((client.client_fd = client.socket->acceptConnection()) < 0)
    {
        Logger::log(RED, ERROR, "Socket could not accept connection: %s", strerror(errno));
        return ;
    }
    client.client_address = client.socket->getSocketAddress();
    client.last_msg_time = time(NULL);

    client.request.setSocket(client.socket);
    client.request.setServerBlocks(_server_blocks);

    //  adding client_fd to epoll instance
    client.epoll_data = addToEpollInstanceEpollin(_epoll_fd, client.client_fd, -1);
    if (client.epoll_data == NULL)
    {
        Logger::log(RED, ERROR, "adding fd[%i] to epoll instance failed", client.client_fd);
        return ;
    }

    // adding new client to _client_map and delete old client with same fd if necessary
    if (_client_map.count(client.client_fd))
        _client_map.erase(client.client_fd);
    _client_map.insert(std::make_pair(client.client_fd, client));

    Logger::log(CYAN, INFO, "Accpted new connection on fd[%i] from address[%s]", client.client_fd, inAddrToIpString(client.client_address.sin_addr.s_addr).c_str());
}

/*
closes connection:
    - removing the client_fd fromt the epoll instance
    - closing the client_fd
    - removing the client from the client_map
*/
void    ServerManager::_closeConnection(int fd)
{
    if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL) < 0)
        Logger::log(RED, ERROR, "Deleting fd[%i] from epoll instance failed: %s", fd, strerror(errno));
    if (close(fd))
        Logger::log(RED, ERROR, "Closing fd[%i] failed: %s", fd, strerror(errno));
    if (_client_map[fd].epoll_data != NULL)
        delete _client_map[fd].epoll_data;
    _client_map.erase(fd);
    Logger::log(CYAN, INFO, "Closed connection on fd[%i]", fd);
}

/*
checking if cgi child process finished 
or check for CGI_TIMEOUT and kill child
*/
void    ServerManager::_checkCgiTimeout(Client &client)
{
    if (client.response.cgi.finished_execution == true)
        return;
    if (difftime(time(NULL), client.response.cgi.getStartTime()) > CGI_TIMEOUT)
    {
        kill(client.response.cgi.getCgiPid(), SIGKILL);
        client.response.cgi.setError(GATEWAY_TIMEOUT);
        client.response.cgi.finished_execution = true;
    }
    else
    {
        int status;
        if (waitpid(client.response.cgi.getCgiPid(), &status, WNOHANG) == 0)
            return;
	    else if (WIFEXITED(status))
        {
            int exit_status = WEXITSTATUS(status);
            if (exit_status != 0)
            {
                client.response.cgi.setError(INTERNAL_SERVER_ERROR);
                Logger::log(RED, ERROR, "Error in CGI script");
            }
        }
        client.response.cgi.finished_execution = true;
    }
    return;
}

/*
checking for timeouts of all clients in the _client_map
*/
void    ServerManager::_checkTimeout()
{
    std::vector<int> timeouts;

    for (std::map<int, Client>::iterator it = _client_map.begin(); it != _client_map.end(); it++)
    {
        if (time(NULL) - it->second.last_msg_time > CLIENT_CONNECTION_TIMEOUT)
            timeouts.push_back(it->first);
    }
    for (size_t i = 0; i < timeouts.size(); i++)
    {
        Logger::log(CYAN, INFO, "Client timeout: Client_FD[%i], closing connection ...", timeouts[i]);
        _closeConnection(timeouts[i]);
    }
}

/*
Reading of the HTTP Request:
    - reading READ_SIZE amount of octest from the client into an buffer
    - parsing the buffer into an HttpRequest object
    - set epoll settings to EPOLLOUT on client_fd if recieved full request
*/
void    ServerManager::_readRequest(Client &client, struct epoll_event event)
{
    uint8_t buffer[READ_BUFFER_SIZE];
    int     bytes_read = 0;
    int     fd = client.client_fd;

    // reading request
    bytes_read = read(fd, buffer, READ_BUFFER_SIZE);
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
        client.last_msg_time = time(NULL);
        client.request.parse(buffer, bytes_read);
        std::memset(buffer, 0, sizeof(buffer));
    }

    // checking if request is fully read
    if (client.request.getParsingState() == Parsing_Finished || client.request.getError() != OK)
    {
        Logger::log(GREEN, INFO, "Request received from client fd[%i] with method[%s] and URI[%s]", fd, client.request.getMethodStr().c_str(), client.request.getPath().c_str());
        if (client.request.getServerBlock() == NULL)
        {
            Logger::log(RED, ERROR, "Could not find an Server to serve with on fd[%i]", fd);
            _closeConnection(fd);
            return ;
        }
        client.response.buildResponse(client.request, client.client_fd, client.client_address);
        if (client.response.cgi_state != No_Cgi)
        {
            if (client.response.cgi.pipe_in[1] != -1)
            {
                client.response.cgi.epoll_data_in = addToEpollInstanceEpollout(_epoll_fd, client.response.cgi.pipe_in[1], client.client_fd);
                if (client.response.cgi.epoll_data_in == NULL)
                {
                    Logger::log(RED, ERROR, "adding fd[%i] to epoll instance failed", client.client_fd);
                    return ;
                }
            }
            client.response.cgi.epoll_data_out = addToEpollInstanceEpollin(_epoll_fd, client.response.cgi.pipe_out[0], client.client_fd);
            if (client.response.cgi.epoll_data_out == NULL)
            {
                Logger::log(RED, ERROR, "adding fd[%i] to epoll instance failed", client.client_fd);
                return ;
            }
        }
        event.events = EPOLLOUT;
        if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, fd, &event))
        {
            Logger::log(RED, ERROR, "Changing settings associated with fd[%i] in epoll instance failed", fd);
            _closeConnection(fd);
            return ;
        }
    }
}

/*
sending the Response to the client:
    - write RESPONSE_WRITE_SIZE of the response to client_fd until full response is send
    - triming the response the amount which got send to the client
    - checking if connection should be "keep-alive"
    - set epoll settings on client_fd to EPOLLIN
    - clearing reuquest and response objects of the client
*/
void    ServerManager::_sendResponse(Client &client, struct epoll_event event)
{
    if (client.response.cgi_state != No_Cgi && client.response.cgi_state != Cgi_Done)
    {
        _checkCgiTimeout(client);
        return;
    }

    int bytes_send = 0;
    int fd = client.client_fd;

    const std::string &response = client.response.getResponse();

    if (response.size() >= WRITE_BUFFER_SIZE)
        bytes_send = write(fd, response.c_str(), WRITE_BUFFER_SIZE);
    else
        bytes_send = write(fd, response.c_str(), response.size());
    if (bytes_send < 0)
    {
        Logger::log(CYAN, INFO, "Could not write on fd[%i]: client closed Connection", fd);
        _closeConnection(fd);
        return ;
    }

    // checking if full response got send
    if (bytes_send == 0 || (size_t)bytes_send == response.size())
    {
        Logger::log(MAGENTA, INFO, "Response send to client fd[%i] with code[%i]", client.client_fd, client.response.getError());
    
        // checking if connection should be "keep-alive"
        if (client.response.checkConnection())
        {
            event.events = EPOLLIN;
            if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, fd, &event))
            {
                Logger::log(RED, ERROR, "Changing settings associated with fd[%i] in epoll instance failed", fd);
                _closeConnection(fd);
                return ;
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

/*
...
*/
void ServerManager::_writeBodyToCgi(Client &client, int pipe_fd)
{
    int bytes_sent;
    std::string &req_body = client.request.getBody();

    if (req_body.empty())
        bytes_sent = 0;
    else if (req_body.length() >= WRITE_BUFFER_SIZE)
        bytes_sent = write(pipe_fd, req_body.c_str(), WRITE_BUFFER_SIZE);
    else
        bytes_sent = write(pipe_fd, req_body.c_str(), req_body.length());

    if (bytes_sent < 0)
    {
        Logger::log(RED, ERROR, "Write Error: failed to write request body to cgi pipe_in: %s", strerror(errno));
        if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, pipe_fd, NULL) < 0)
            Logger::log(RED, ERROR, "Deleting fd[%i] from epoll instance failed: %s", pipe_fd, strerror(errno));
        close(client.response.cgi.pipe_in[1]);
        client.response.cgi.setError(INTERNAL_SERVER_ERROR);
        client.response.cgi_state = Cgi_Read;
    }
    else if (bytes_sent == 0 || (size_t) bytes_sent == req_body.length())
    {
        if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, pipe_fd, NULL) < 0)
            Logger::log(RED, ERROR, "Deleting fd[%i] from epoll instance failed: %s", pipe_fd, strerror(errno));
        close(client.response.cgi.pipe_in[1]);
        client.response.cgi_state = Cgi_Read;
    }
    else
        req_body = req_body.substr(bytes_sent);
}

/*
...
*/
void ServerManager::_readCgiResponse(Client &client, int pipe_fd)
{
    uint8_t buffer[READ_BUFFER_SIZE];
    int     bytes_read = 0;

    bytes_read = read(pipe_fd, buffer, READ_BUFFER_SIZE);

    if (bytes_read < 0)
    {
        Logger::log(RED, ERROR, "read error on cgi output: ", strerror(errno));
        if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, pipe_fd, NULL) < 0)
            Logger::log(RED, ERROR, "Deleting fd[%i] from epoll instance failed: %s", pipe_fd, strerror(errno));
        close(client.response.cgi.pipe_out[0]);
        client.response.cgi_state = Cgi_Done;
        client.response.cgi.setError(INTERNAL_SERVER_ERROR);
        client.response.constructResponseStr(client.request);
    }
    else if (bytes_read == 0)
    {
        if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, pipe_fd, NULL) < 0)
            Logger::log(RED, ERROR, "Deleting fd[%i] from epoll instance failed: %s", pipe_fd, strerror(errno));
        close(client.response.cgi.pipe_out[0]);
        client.response.cgi_state = Cgi_Done;
        client.response.constructResponseStr(client.request);
    }
    else
    {
        client.response.cgi.parseCgi(buffer, bytes_read);
		memset(buffer, 0, sizeof(buffer));
    }
}

// ==========   Member functions   =========== //

/*
setting up all servers
    - calls parsing of the config file
    - setting up all sockets
    - adding sockets to the _socket_map
    - assigning sockets to the server blocks
*/
void    ServerManager::setup(std::string config)
{
    Logger::log(WHITE, INFO, "Setting up Servers ...");

    // parsing the config file 
    ConfigParser        parser(_server_blocks);

    parser.parse(config);
    Logger::log(GREY, DEBUG, "Finished config file parsing");
    if (_server_blocks.size() == 0)
    {
        Logger::log(RED, ERROR, "Config File: no server block found ( empty file ? )");
        exit(EXIT_FAILURE);
    }

    // printing the server setup
    for (size_t i = 0; i < _server_blocks.size(); i++)
    {
        std::string server_name;
        if (_server_blocks[i].server_names.empty())
            _server_blocks[i].server_names.push_back(DEFAULT_NAME);
        if (!_server_blocks[i].server_names.empty())
            server_name = _server_blocks[i].server_names[0];
        Logger::log(WHITE, INFO, "Server setup: Name[%s] Host[%s] Port[%i]", server_name.c_str(), _server_blocks[i].ip.c_str(), _server_blocks[i].port);
    }

    // find all needed sockets
    std::map<uint16_t, in_addr_t> map;

    for (size_t i = 0; i < _server_blocks.size(); i++)
        map.insert(std::pair<uint16_t, in_addr_t>(_server_blocks[i].port, _server_blocks[i].host));

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
        if (sockets[i].setup())
        {
            Logger::log(RED, ERROR, "Could not setup socket: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
        _socket_map.insert(std::pair<int, Socket>(sockets[i].getFd(), sockets[i]));
        Logger::log(GREY, DEBUG, "Socket setup: Host[%s] Port[%i]", inAddrToIpString(htonl(sockets[i].getHost())).c_str(), sockets[i].getPort());
    }

    // assigning sockets to server blocks
    for (size_t i = 0; i < _server_blocks.size(); i++)
    {
        for (std::map<int, Socket>::iterator it = _socket_map.begin(); it != _socket_map.end(); it++)
        {
            if (it->second.getHost() == _server_blocks[i].host && it->second.getPort() == _server_blocks[i].port)
            {
                _server_blocks[i].socket = &it->second;
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
        if (addListeningSocketToEpoll(_epoll_fd, it->second.getFd(), -1) < 0)
        {
            Logger::log(RED, ERROR, "adding fd[%i] to epoll instance failed", it->second.getFd());
            exit(EXIT_FAILURE);
        }
        if (it->second.startListening() < 0)
        {
            Logger::log(RED, ERROR, "Socket could not listen: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
        
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
            Logger::log(RED, ERROR, "Waiting for event on the epoll instance failed: %s", strerror(errno));
            // Interrupted by a signal; retry
            if (errno == EINTR)
                continue ;
            else
                exit(EXIT_FAILURE);
        }
        // handling of the epoll event list
        for (int i = 0; i < num_events; i++)
        {
            int fd = static_cast<e_data*>(event_list[i].data.ptr)->fd;
			int client_fd = static_cast<e_data*>(event_list[i].data.ptr)->client_fd;

            if (_socket_map.count(fd))
                _acceptNewConnection(fd);
            else if (_client_map.count(fd))
            {
                if (event_list[i].events & EPOLLERR || event_list[i].events & EPOLLHUP || event_list[i].events & EPOLLRDHUP)
                    _closeConnection(fd);
                else if (event_list[i].events & EPOLLIN)
                    _readRequest(_client_map[fd], event_list[i]);
                else if (event_list[i].events & EPOLLOUT)
                    _sendResponse(_client_map[fd], event_list[i]);
            }
            else if (_client_map.count(client_fd))
            {
                if (_client_map[client_fd].response.cgi_state == Cgi_Write)
                    _writeBodyToCgi(_client_map[client_fd], fd);
                else if (_client_map[client_fd].response.cgi_state == Cgi_Read && _client_map[client_fd].response.cgi.finished_execution == true)
                    _readCgiResponse(_client_map[client_fd], fd);
            }
        }
        _checkTimeout();
    }
}
