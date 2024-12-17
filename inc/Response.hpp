#pragma once

#include "Webserv.hpp"

struct CgiReturn {
    int fdread;
    char **newenv;
};

class Response
{
    private:
        int         _error;
        std::string _response;
        std::string _connection;
        std::string _content;
        std::string _content_type;
        std::string _location;
        int         _clientfd;

    // Private member functions 
        void        _setConnection(Request& request);
        void        _setErrorPage(ServerBlock &server);
        void        _buildResponseStr(Request &request, ServerBlock &sever);
        void        _handleGet(Request &request, ServerBlock &server);
        void        _handlePost(Request &request, ServerBlock &server);
        void        _handleDelete(Request &request, ServerBlock &server);
        int          _process_cgi(std::string cgipath, std::string cgi_file, int clientfd, Request &ref1, ServerBlock &ref2);
        char**      _buildenv(const char *cgifile, int clientfd, Request &ref1, ServerBlock &ref2);

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
        void        buildResponse(Request &request, int clientfd);
        void        trimResponse(int i);
        void        clear();
    
};