#include "../inc/ServerManager.hpp"

void    ServerManager::configure(std::string config)
{
    ConfigParser    parser(_servers);

    parser.parse(config);



}

void    ServerManager::setup()
{
    bool    duplicate;

    for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); it++)
    {
        duplicate = false;
        for (std::vector<Server>::iterator it2 = _servers.begin(); it != _servers.end(); it2++)
        {
            if (it->getHost() == it2->getHost() && it->getPort() == it2->getPort())
                duplicate = true;
        }
        if (!duplicate)
            it->setup();
    }
}

void    ServerManager::acceptNewConnection(int server_fd)
{
    int new_socket;

    new_socket = _server_map[server_fd].acceptConnection();
    // add to client map
}

void    ServerManager::boot()
{
    // starts to listen on all servers

    // creates epoll instance
    int _epoll_fd = epoll_create(1);
    if (_epoll_fd = -1)
    {
        std::cerr << "Error creating epoll instance: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    // adds all server_fds to epoll instance
    for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); it++)
    {
        struct epoll_event event;
        event.events = EPOLLIN | EPOLLOUT; 
        event.data.fd = it->getServerFd();
        if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, it->getServerFd(), &event) == -1)
        {
            std::cerr << "Error: epoll_ctl: fd" << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    struct epoll_event event_list[MAX_EPOLL_EVENTS];
    while (true)
    {
        int num_events = epoll_wait(_epoll_fd, event_list, MAX_EPOLL_EVENTS, -1);
        if (num_events == -1)
        {
            std::cerr << "Error: epoll_wait" << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < num_events; i++)
        {
            int fd = event_list[i].data.fd;
            
            if (_server_map.count(fd))
                acceptNewConnection(fd);
            else if (_client_map.count(fd) && event_list[i].events & EPOLLIN)
                readRequest();
            else if (_client_map.count(fd) && event_list[i].events & EPOLLOUT)
                sendResponse();
            else
                close(fd);
        }
        // timeout
    }
}