#pragma once

#include "Webserv.hpp"

#define MAX_URI_LENGHT          4096
#define MAX_HEADER_LENGHT       8192
#define MAX_BODY_SIZE           1048576

enum ParsingState
{
    Empty_Line_CR,
    Empty_Line_LF,
    Request_Line_Method,
    Request_Line_Space,
    Request_Line_URI_Slash,
    Request_Line_URI_Path,
    Request_Line_URI_Query,
    Request_Line_URI_Fragment,
    Request_Line_H,
    Request_Line_HT,
    Request_Line_HTT,
    Request_Line_HTTP,
    Request_Line_HTTP_Slash,
    Request_Line_Version_Major,
    Request_Line_Version_Dot,
    Request_Line_Version_Minor,
    Request_Line_CR,
    Request_Line_LF,
    Header_Field_Start,
    Header_Field_Name,
    Header_Field_Value,
    Header_Field_End,
    Header_Field_Blank_Line,
    Chunk_Lenght,
    Chunk_Extensions,
    Chunk_Lenght_LF,
    Chunk_Data,
    Chunk_Data_CR,
    Chunk_Data_LF,
    Chunk_Last_CR,
    Chunk_Last_LF,
    Chunk_Trailer_Section,
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
    int                                             _error;
    HttpMethod                                      _method;
    std::string                                     _path;
    std::string                                     _query;
    std::string                                     _fragment;
    int                                             _version_major;
    int                                             _version_minor;
    std::map<std::string, std::string>              _headers;
    std::string                                     _body;

// help and storage variables
    std::string                                     _method_str;
    std::string                                     _header_field_name;
    std::string                                     _header_field_value;
    std::string                                     _chunk_lenght_str;
    std::stringstream                               _ss;
    size_t                                          _uri_len;
    size_t                                          _header_len;
    size_t                                          _body_len;
    size_t                                          _chunk_len;

// flags
    bool                                            _body_flag;
    bool                                            _chuncked_transfer_flag;

public:
// Default Constructor
    HttpRequest();

// Deconstructor
    ~HttpRequest();

// Member functions
    void                                parse(char *data, size_t size);

// Getters
    int                                         getError();
    int                                         getVersionMajor();
    int                                         getVersionMinor();
    ParsingState                                getParsingState();
    HttpMethod                                  getMethod();
    const std::string                           &getPath();
    const std::string                           &getQuery();
    const std::string                           &getFragment();
    const std::string                           &getBody();
    const std::map<std::string, std::string>    &getFields();

};

// util functions
bool    allowedURIChar(uint8_t c);
bool    allowedFieldNameChar(uint8_t c);
bool    checkPathUnderRoot(std::string path);
void    trimFieldValueStr(std::string &string);
