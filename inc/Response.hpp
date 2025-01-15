#pragma once

#include "Webserv.hpp"

class Request;

class Response
{
    private:
        int                                 _error;
        std::string                         _response;
        std::string                         _body;
        sockaddr_in                         _client_addr;
        std::map<std::string, std::string>  _headers;

    // Private member functions
        void        _handleRequest(Request &request, ServerBlock &server);
        bool        _checkCgi(Request &request, ServerBlock &server, std::string path, Location &location);
        void        _handleGet(ServerBlock &server, std::string path, Location &location);
        void        _handlePost(Request &request, std::string path, Location &location);
        void        _handleDelete(std::string path);
        void        _setConnection(Request& request);
        void        _buildErrorPage(ServerBlock &server);

    public:
    // Constructor
        Response();
    
    // Decosntructor
        ~Response();
    
    // Getters
        int                 getError() const;
        const std::string&  getResponse() const;

    // Member functions
        void        buildResponse(Request &request, sockaddr_in client_addr);
        bool        checkConnection();
        void        trimResponse(int i);
        void        clear();
    
};

// utils
std::string intToStr(int n);
