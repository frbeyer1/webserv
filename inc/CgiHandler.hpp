#pragma once

#include "Webserv.hpp"

struct e_data 
{
	int	fd;
	int client_fd;
};

enum CgiParsingState
{
    Cgi_Header_Start,
    Cgi_Header_Key,
    Cgi_Header_WS,
    Cgi_Header_Value,
    Cgi_Header_End,
    Cgi_End_LF,
    Cgi_Parsing_Finished,
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
    int                                 _cgi_pid;
    time_t                              _start_time;

// Private Member functions
    bool    _addHeader(std::string &header_name, std::string &header_value);
    void    _buildEnvironment(Request &request, ServerBlock &server, sockaddr_in client_addr);
    void    _closePipes();

public:
// Public member variables
    int                                 pipe_in[2];
    int                                 pipe_out[2];
    bool                                finished_execution;
    e_data*                             epoll_data_in;
    e_data*                             epoll_data_out;

// Constructor
    CgiHandler();

// Deconstructor
    ~CgiHandler();

// Getters
    int                                         getError() const;
    std::string                                 getBody() const;
    const std::map<std::string, std::string>&   getHeaders() const;
    int                                         getCgiPid() const;
    time_t                                      getStartTime() const;

// Setters
    void                setError(int error);

// Member functions
    void    execCgi(Request &request, ServerBlock &server, std::string script_path, std::string binary_path, sockaddr_in client_addr);
    void    parseCgi(uint8_t *data, size_t size);
    void    clear();

};

// utils
std::string intToStr(int n);
