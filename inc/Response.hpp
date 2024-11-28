#pragma once

#include "Webserv.hpp"

#define DEFAULT_type "text/html"

class Response
{
    private:

        std::string _file;
        std::string _contentPath;
        std::string _response_str;
        std::string _content;
        std::string _contentType;
        size_t      _contentLength;
        size_t      _code;

        // MIME TYPE??
        std::string _buildDefaultErrorPage(int error_code);
        std::string _GETmethod(HttpRequest &request);
        std::string _POSTmethod(HttpRequest &request);
        std::string _DELETEmethod();

    public:
        Response();
        ~Response();
        void        buildResponse(HttpRequest &request);
        size_t        checkContent();
    
        std::string getTargetFile();
        std::string getType(HttpRequest &request);
        std::string getTimeAndDate();
        std::string &getResponseStr();
        size_t      getCode();
};