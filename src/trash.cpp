#include "Webserv.hpp"

// int main()
// {
//     Server server;

//     server.launch();
// }

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

// /*
// function to create an tcp server socket
// */
// int create_tcp_server_socket()
// {
//     // 1. create the socket
//     int fd = socket(AF_INET, SOCK_STREAM, 0);
//     if (fd < 0)
//     {
//         std::cerr << "Error: Could not set up socket" << std::endl;
//         exit(EXIT_FAILURE);
//     }
//     // 2. sets the socket to reuse ports
//     const int opt = 1;
//     if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
//     {
//         std::cerr << "setsockopt(SO_REUSEADDR) failed" << std::endl;
//         exit(EXIT_FAILURE);
//     }
//     // 3. setup and bind the address to the socket
//     struct sockaddr_in  address;
//     address.sin_family = AF_INET;
//     address.sin_port = htons(80);
//     address.sin_addr.s_addr = htonl(INADDR_ANY);
//     memset(address.sin_zero, '\0', sizeof(address.sin_zero));
//     if (bind(fd, (struct sockaddr *)&address, sizeof(address)) < 0)
//     {
//         std::cerr << "Error: Could not bind socket" << std::endl;
//         exit(EXIT_FAILURE);
//     }
//     // 4. start listening to incoming connections
//     if (listen(fd, 3) < 0)
//     {
//         std::cerr << "Error: Socket could not listen" << std::endl;
//         exit(EXIT_FAILURE);
//     }
//     return fd;
// }

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

// typedef struct location_s
// {
//     std::vector<HttpMethod>     allowed_methods;
//     std::string                 redirection;
//     std::string                 alias;
//     std::string                 index;
//     std::string                 upload;
//     bool                        autoindex;

// }           location_t;

// typedef struct server_block_s
// {
//     in_addr_t                   host;
//     uint16_t                    port;
//     std::string                 server_name;
//     std::string                 root;
//     std::map<int, std::string>  error_pages;
//     size_t                      max_client_body_size;
//     std::vector<location_t>     locations;
// }       ServerBlock_t;

// class Config
// {
// private:
//     std::string                 _host;
//     uint16_t                    _port;
//     std::string                 _server_name;
//     std::map<int, std::string>  _error_pages;
//     size_t                      _max_client_body_size;
//     std::string                 _root;
//     std::string                 _index;




//     // std::string						_index;
//     // bool                        _auto_index;
//     // std::vector<Location>       _locations;

// public:
// // Constructor
//     Config();

// // Deconstructor
//     ~Config();

// };


