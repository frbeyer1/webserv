#pragma once

#include "Webserv.hpp"

class Response
{
    private:

        std::string _file;
        std::string _contentPath;
        std::string _response_str;
        std::string _content;
        std::string _contentType;
        size_t      _contentLenght;
        size_t      _code;

        // MIME TYPE??
        std::string _buildDefaultErrorPage(int error_code);
        std::string _GETmethod();
        std::string _POSTmethod();
        std::string _DELETEmethod();

    public:
        Response();
        ~Response();
        void        buildResponse(HttpRequest &request);
        void        checkContent();
    
        std::string getTargetFile();
        std::string getType();
        std::string getTimeAndDate();
        std::string &getResponseStr();
};