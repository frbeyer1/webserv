#pragma once

#include "Webserv.hpp"

enum ParsingState
{
    Request_Line_Method,
    Request_Line_First_Space,
    Request_Line_URI_Path_Slash,
    Request_Line_URI_Path,
    Request_Line_URI_Query,
    Request_Line_URI_Fragment,
    Request_Line_Version_H,
    Request_Line_Version_HT,
    Request_Line_Version_HTT,
    Request_Line_Version_HTTP,
    Request_Line_Version_HTTP_Slash,
    Request_Line_Version_Major,
    Request_Line_Version_Dot,
    Request_Line_Version_Minor,
    Request_Line_CR,
    Request_Line_LF,
    Header_Field_Name_Start,
    Header_Field_Name,
    Header_Field_Value,
    Header_Field_LF,
    Header_Field_Blank_Line,




    Parsing_Finished,
};

enum HttpMethod
{
    GET,
    POST,
    DELETE,
    NONE,
};


/*
...
*/
class HttpRequest
{
private:
    ParsingState                        _state;
    short                               _error;
    HttpMethod                          _method;
    std::string                         _path;
    std::string                         _query;
    std::string                         _fragment;
    short                               _version_major;
    short                               _version_minor;
    std::map<std::string, std::string>  _header_fields;
    std::string                         _header_field_name;
    std::string                         _header_field_value;
    std::string                         _method_str;
    size_t                              _method_str_len;

    bool    allowedURIChar(char c);

public:
// Default Constructor
    HttpRequest();

// Deconstructor
    ~HttpRequest();

// Member functions
    void    parse(char character);

// Getters


};