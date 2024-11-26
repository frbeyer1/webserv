#pragma once

#include "Webserv.hpp"

class Response
{
    private:
        std::string _file;

        std::string _response_str;
        std::string _content;
        std::string _contentType;
        size_t      _contentLenght;
        size_t      _code;

        Server      _server; // ??
        // MIME TYPE??
        std::string _buildDefaultErrorPage(int error_code);
        std::string _GETResponse();
        // POST
        // DELETE
        // std::string _buildContent();
    public:
        Response();
        ~Response();
        void    build(HttpRequest &request);
        std::string getTargetFile();
        size_t      getCode();
        std::string getType();
        std::string &getResponseStr();
};