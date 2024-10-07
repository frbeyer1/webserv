#pragma once

#include "Webserv.hpp"

#define DEFAULT_PORT                    80
#define DEFAULT_NAME                    "default"
#define DEFAULT_ROOT                    "docs/"
#define DEFAULT_CLIENT_MAX_BODY_SIZE    1048576

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

public:
    Server();

    void clear();
    void setup();

    void setRoot(std::string parameter);
    void setListen(std::string parameter);
    void setServerName(std::string parameter);
    void setClientMaxBodySize(std::string parameter);
    void setErrorPage(std::string parameter);
    // setLocation();


    in_addr_t   getHost() const;
    uint16_t    getPort() const;
    std::string getSeverName() const;
    std::string getRoot() const;
    size_t      getClientMaxBodySize() const;

};
