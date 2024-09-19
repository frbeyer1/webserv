#pragma once

#include "Webserv.hpp"

#define MAX_URI_LENGHT              4096

enum ParsingState
{
    Method,
    Space,
    URI_Path_Slash,
    URI_Path,
    URI_Query,
    URI_Fragment,
    Version_H,
    Version_HT,
    Version_HTT,
    Version_HTTP,
    Version_HTTP_Slash,
    Version_Major,
    Version_Dot,
    Version_Minor,
    Request_Line_CR,
    Request_Line_LF,
    Field_Start,
    Field_Name,
    Field_Value,
    Field_End,
    Field_Blank_Line,
    Chuncked_Lenght,
    Chuncked_Lenght_CR,
    Chuncked_Lenght_LF,
    Chuncked_Data,
    Chuncked_Data_CR,
    Chuncked_Data_LF,
    Message_Body,
    Parsing_Finished,
};

enum HttpMethod
{
    GET,
    POST,
    DELETE,
    NONE,
};

class HttpRequest
{
private:
    ParsingState                                    _state;
    short                                           _error;
    HttpMethod                                      _method;
    std::string                                     _path;
    std::string                                     _query;
    std::string                                     _fragment;
    short                                           _version_major;
    short                                           _version_minor;
    std::map<std::string, std::string>              _fields;
    std::string                                     _body;


    std::map<std::string, std::string>::iterator    _it;
    std::string                                     _header_field_name;
    std::string                                     _header_field_value;
    std::string                                     _method_str;
    size_t                                          _method_str_len;
    size_t                                          _uri_len;
    size_t                                          _request_len;
    size_t                                          _body_lenght;
    bool                                            _body_flag;
    bool                                            _chuncked_transfer_flag; 

public:
// Default Constructor
    HttpRequest();

// Deconstructor
    ~HttpRequest();

// Member functions
    void    parse(char *data, size_t size);

// Getters


};

bool    allowedURIChar(char c);
bool    allowedFieldNameChar(char c);
bool    checkPathUnderRoot(std::string path);
void    trimFieldValueStr(std::string &string);
