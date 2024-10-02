#include "Server.hpp"

// =============   Constructor   ============= //
Server::Server()
{
    // 1. create the socket
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd < 0)
    {
        std::cerr << "Error: Could not set up socket" << std::endl;
        exit(EXIT_FAILURE);
    }
    // sets the socket to reuse ports
    _reuseable_ports = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &_reuseable_ports, sizeof(int)) < 0)
    {
        std::cerr << "setsockopt(SO_REUSEADDR) failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    // 2. setup and bind the address to the socket
    _address.sin_family = AF_INET;
    _address.sin_port = htons(80);
    _address.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(_address.sin_zero, '\0', sizeof(_address.sin_zero));
    if (bind(_fd, (struct sockaddr *)&_address, sizeof(_address)) < 0)
    {
        std::cerr << "Error: Could not bind socket" << std::endl;
        exit(EXIT_FAILURE);
    }
    // 3. start listening to incoming connections
    if (listen(_fd, 3) < 0)
    {
        std::cerr << "Error: Socket could not listen" << std::endl;
        exit(EXIT_FAILURE);
    }
}

// ============   Deconstructor   ============ //
Server::~Server()
{

}

// Function to handle HTTP response
std::string getHttpResponse() 
{
    std::ifstream file("test.html");
    if (!file.is_open())
    {
        std::cerr << "Could not open file: " << "test.html" << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\n";
    response << "Content-Type: text/html\n";
    response << "Content-Length: " << buffer.str().length() << "\n";
    response << std::endl; // End of headers
    response << buffer.str();   // The actual body
    return response.str();
}

/*
function to set an fd into non-blocking mode
*/
void    setNonBlocking(int fd)
{
    // Get the current flags for the file descriptor
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        std::cerr << "Error getting flags for file descriptor" << std::endl;
        exit(EXIT_FAILURE);
    }
    // Set the flags to include O_NONBLOCK
    flags |= O_NONBLOCK;
    // Set the new flags for the file descriptor
    if (fcntl(fd, F_SETFL, flags) == -1)
    {
        std::cerr << "Error setting flags for file descriptor" << std::endl;
        exit(EXIT_FAILURE);
    }
}

// ==========   Member functions   =========== //
void    Server::launch()
{
    int addrlen = sizeof(_address);
    int new_socket;
    long valread;

    while(1)
    {
        std::cout << "===== Waiting =====" << std::endl;
        if ((new_socket = accept(_fd, (struct sockaddr *)&_address, (socklen_t*)&addrlen)) < 0)
        {
            std::cerr << "Error: Socket could not accept new connection" << std::endl; 
            exit(EXIT_FAILURE);
        }
        char buffer[30000] = {0};
        valread = read( new_socket , buffer, 30000);
        std::cout << buffer << std::endl;

        std::string response = getHttpResponse();

        write(new_socket , response.c_str() , response.size());
        std::cout << "===== Done =====" << std::endl;
        close(new_socket);
    }
    close(_fd);
    (void)valread;
}

