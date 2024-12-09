#pragma once

#include "Webserv.hpp"

class Response
{
    private:
        int         _error;
        std::string _response;
        std::string _connection;
        std::string _content;
        std::string _content_type;
        std::string _location;

    // Private member functions 
        void    _setConnection(HttpRequest& request);
        void    _setErrorPage(Server &server);
        void    _buildResponseStr(HttpRequest &request, Server &sever);
        void    _handleGet(HttpRequest &request, Server &server);
        void    _handlePost(HttpRequest &request, Server &server);
        void    _handleDelete(HttpRequest &request, Server &server);

    public:
    // Constructor
        Response();
    
    // Decosntructor
        ~Response();
    
    // Getters
        int                 getError() const;
        const std::string   &getResponse() const;
        const std::string   &getConnection() const;

    // Member functions
        void        buildResponse(HttpRequest &request, Server &server);
        void        trimResponse(int bytes_send);
        void        clear();
    
};