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

    // Private member functions 
        void        _setConnection(Request& request);
        void        _setErrorPage(ServerBlock &server);
        void        _buildResponseStr(Request &request, ServerBlock &sever);
        void        _handleGet(Request &request, ServerBlock &server);
        void        _handlePost(Request &request, ServerBlock &server);
        void        _handleDelete(Request &request, ServerBlock &server);
        CgiReturn*  _process_cgi(int cgifd, char *cgifile, char **env, int clientfd, Request &ref1, ServerBlock &ref2);
        char**      _buildenv(char *cgifile, char **env,int clientfd, Request &ref1, ServerBlock &ref2);

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
        void        buildResponse(Request &request);
        void        trimResponse(int i);
        void        clear();
    
};