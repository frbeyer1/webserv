#include "Server.hpp"

Server::Server()
{
    // _host = ;
    _port = DEFAULT_PORT;
    _server_name = DEFAULT_NAME;
    _root = DEFAULT_ROOT;
    _client_max_body_size = DEFAULT_CLIENT_MAX_BODY_SIZE;
    // _error_pages = ;
    // _locations = ;
}

void Server::clear()
{
    // _host = ;
    _port = DEFAULT_PORT;
    _server_name = DEFAULT_NAME;
    _root = DEFAULT_ROOT;
    _client_max_body_size = DEFAULT_CLIENT_MAX_BODY_SIZE;
    // _error_pages = ;
    // _locations = ;
}

void    Server::setRoot(std::string parameter)
{


}

void    Server::setListen(std::string parameter)
{
    

}

void    Server::setServerName(std::string parameter)
{
    

}

void    Server::setClientMaxBodySize(std::string parameter)
{
    

}

void    Server::setErrorPage(std::string parameter)
{


}

in_addr_t   Server::getHost() const
{
    return (_host);
}

uint16_t    Server::getPort() const
{
    return (_port);
}

std::string Server::getSeverName() const
{
    return (_server_name);
}

std::string Server::getRoot() const
{
    return (_root);
}

size_t      Server::getClientMaxBodySize() const
{
    return (_client_max_body_size);
}

/*
function to set an fd into non-blocking mode
*/
// void    set_non_blocking(int fd)
// {
//     // Get the current flags for the file descriptor
//     int flags = fcntl(fd, F_GETFL, 0);
//     if (flags == -1)
//     {
//         std::cerr << "Error getting flags for file descriptor" << std::endl;
//         exit(EXIT_FAILURE);
//     }
//     // Set the flags to include O_NONBLOCK
//     flags |= O_NONBLOCK;
//     // Set the new flags for the file descriptor
//     if (fcntl(fd, F_SETFL, flags) == -1)
//     {
//         std::cerr << "Error setting flags for file descriptor" << std::endl;
//         exit(EXIT_FAILURE);
//     }
// }

/*
function to create and setup an tcp server socket
*/
void    Server::setup()
{
    // 1. create the socket
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd < 0)
    {
        std::cerr << "Error: Could not set up socket" << std::endl;
        exit(EXIT_FAILURE);
    }
    // 2. sets the socket to reuse ports
    const int opt = 1;
    if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        std::cerr << "setsockopt(SO_REUSEADDR) failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    // 3. setup and bind the address to the socket
    struct sockaddr_in  address;
    address.sin_family = AF_INET;
    address.sin_port = htons(_port);
    address.sin_addr.s_addr = htonl(_host);
    memset(address.sin_zero, '\0', sizeof(address.sin_zero));
    if (bind(_server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << "Error: Could not bind socket" << std::endl;
        exit(EXIT_FAILURE);
    }
}


//     // 4. start listening to incoming connections
//     if (listen(fd, 3) < 0)
//     {
//         std::cerr << "Error: Socket could not listen" << std::endl;
//         exit(EXIT_FAILURE);
//     }


// #define MAX_EVENTS  10

// int main()
// {
//     int server_fd = create_tcp_server_socket();

//     set_non_blocking(server_fd);

//     // create epoll instance
//     struct epoll_event event;
//     int epoll_fd = epoll_create(1);
//     if (epoll_fd = -1)
//     {
//         std::cerr << "Error creating epoll instance: " << strerror(errno) << std::endl;
//         exit(EXIT_FAILURE);
//     }
//     // adds the fd to the epoll event
// 	struct epoll_event event;
//     event.events = EPOLLIN | EPOLLOUT; 
//     event.data.fd = server_fd;
//     // adds server_fd to fd_list
//     if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1)
//     {
//         std::cerr << "Error: epoll_ctl: fd" << strerror(errno) << std::endl;
//         exit(EXIT_FAILURE);
//     }
//     struct epoll_event event_list[MAX_EVENTS];
//     while (1)
//     {
//         int num_events = epoll_wait(epoll_fd, event_list, MAX_EVENTS, -1);
//         if (num_events == -1)
//         {
//             std::cerr << "Error: epoll_wait" << strerror(errno) << std::endl;
//             exit(EXIT_FAILURE);
//         }
//         for (int i = 0; i < num_events; i++)
//         {
//             int fd = event_list[i].data.fd;
//             if (fd == server_fd)
//             {
//                 /* New connection request received */
//                 accept_new_connection_request(fd);
//             }
//             else if ((event_list[i].events & EPOLLERR) || 
//                         (event_list[i].events & EPOLLHUP) || 
//                         (!(event_list[i].events & EPOLLIN)))
//             {
//                 /* Client connection closed */
//                 close(fd);
//             }
//             else
//             {
//                 /* Received data on an existing client socket */
//                 recv_and_forward_message(fd);
//             }
//         }
//     }
// }





// uint32_t ipStringToNumeric(const std::string& ip) {
//     std::stringstream ss(ip);
//     std::string segment;
//     uint32_t numericIp = 0;
//     int segmentCount = 0;

//     while (std::getline(ss, segment, '.')) {
//         if (segmentCount >= 4) {
//             throw std::invalid_argument("Invalid IP address format");
//         }
//         int segmentValue = std::stoi(segment);
//         if (segmentValue < 0 || segmentValue > 255) {
//             throw std::invalid_argument("IP address segments must be between 0 and 255");
//         }
//         numericIp = (numericIp << 8) | segmentValue; // Shift left by 8 and add segment
//         segmentCount++;
//     }

//     if (segmentCount != 4) {
//         throw std::invalid_argument("Invalid IP address format: must have 4 segments");
//     }
    
//     return numericIp;
// }