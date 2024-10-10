#pragma once

#include "Webserv.hpp"

typedef struct location_s
{
    std::vector<HttpMethod>     allowed_methods;
    std::string                 redirection;
    std::string                 alias;
    std::string                 index;
    // std::string                 upload;
    bool                        autoindex;

}           location_t;

class Server
{
private:
    in_addr_t                   _host;
    uint16_t                    _port;
    std::string                 _server_name; 
    std::string                 _root;
    size_t                      _client_max_body_size;
    std::map<int, std::string>  _error_pages;
    std::vector<location_t>     _locations;
    int                         _server_fd;
    struct sockaddr_in          _socket_address;

public:
// Constructor
    Server();

// Deconstructor
    ~Server();

// Getters
    in_addr_t   getHost() const;
    uint16_t    getPort() const;
    int         getServerFd() const;
    std::string getSeverName() const;
    std::string getRoot() const;
    size_t      getClientMaxBodySize() const;

// Setters
    void        setRoot(std::string root);
    void        setPort(uint16_t port);
    void        setHost(in_addr_t host);
    void        setServerName(std::string server_name);
    void        setClientMaxBodySize(size_t client_max_body_size);
    void        setErrorPage(std::string parameter);
    void        setLocation(location_t location);

// Member functions
    void        setup();
    void        startListening();
    int         acceptConnection(); 

};
