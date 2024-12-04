#pragma once

#include "Webserv.hpp"

typedef struct allowed_methods_s
{
    bool        allow_get;
    bool        allow_delete;
    bool        allow_post;
}       allowed_methods_t;

typedef struct location_s
{
    allowed_methods_t                   allowed_methods;
    std::string                         redirection;
    std::string                         alias;
    std::string                         index;
    std::string                         upload;
    std::map<std::string, std::string>  cgi;
    bool                                autoindex;
}           location_t;

class Server
{
private:
    in_addr_t                           _host;
    uint16_t                            _port;
    std::string                         _server_name;
    std::string                         _ip;
    std::string                         _root;
    size_t                              _client_max_body_size;
    std::map<int, std::string>          _error_pages;
    std::map<std::string, location_t>   _locations;
    int                                 _server_fd;
    struct sockaddr_in                  _socket_address;

public:
// Constructor
    Server();

// Deconstructor
    ~Server();

// Getters
    in_addr_t                               getHost() const;
    std::string                             getIp() const;
    uint16_t                                getPort() const;
    int                                     getServerFd() const;
    std::string                             getServerName() const;
    std::string                             getRoot() const;
    size_t                                  getClientMaxBodySize() const;
    std::map<int, std::string>&             getErrorPages() ;
    struct sockaddr_in                      getSocketAddress() const;
    const std::map<std::string, location_t> &getLocations() const; //new

// Setters
    void                          setRoot(std::string root);
    void                          setPort(uint16_t port);
    void                          setHost(in_addr_t host);
    void                          setIp(std::string ip);
    void                          setServerName(std::string server_name);
    void                          setClientMaxBodySize(size_t client_max_body_size);
    void                          setErrorPage(int status_code, std::string page_path);
    void                          setLocation(std::string path, location_t location);

// Member functions
    void                          setup();
    void                          startListening();
    int                           acceptConnection(); 

};
