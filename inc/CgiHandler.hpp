#pragma once

#include "Webserv.hpp"
#include "Response.hpp"

enum CgiParsingState
{
    CGI_HEADER_START,
    CGI_HEADER_KEY,
    CGI_HEADER_WS,
    CGI_HEADER_VALUE,
    CGI_HEADER_END,
    CGI_PARSING_FINISHED,
};

class CgiHandler
{
private:
    CgiParsingState                     _state;
    int                                 _error;
    std::map<std::string, std::string>  _headers;
    std::string                         _body;
    char**                              _env;
    std::string                         _binary_path;
    std::string                         _script_path;
    sockaddr_in                         _client_addr;
    Request&                            _request;
    ServerBlock&                        _server;

// Private Member functions
    void    _readCgi(int fd);
    void    _parseCgi(std::string &output);
    bool    _addHeader(std::string &header_name, std::string &header_value);
    void    _buildEnvironment();

public:
// Constructor
    CgiHandler(Request &request, ServerBlock &server, std::string script_path, std::string binary_path, sockaddr_in client_addr);

// Deconstructor
    ~CgiHandler();

// Getters
    int                                         getError() const;
    std::string                                 getBody() const;
    const std::map<std::string, std::string>&   getHeaders() const;

// Member functions
    void    execCgi();

};
