#pragma once

#include "Webserv.hpp"

#define CR                                          '\r'
#define LF                                          '\n'

enum HttpMethod
{
    NONE,
    GET,
    POST,
    DELETE,
};

enum ParsingState
{
    Empty_Line,
    Request_Line_Method,
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
    Chunk_Length,
    Chunk_Extensions,
    Chunk_Length_End,
    Chunk_Data,
    Chunk_Data_CR,
    Chunk_Data_LF,
    Chunk_Last_CR,
    Chunk_Last_LF,
    Chunk_Trailer_Section,
    Message_Body,
    Parsing_Finished,
};

class Socket;

class Request
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
    std::string                                     _method_str;
    std::string                                     _header_field_name;
    std::string                                     _header_field_value;
    std::string                                     _chunk_length_str;
    std::stringstream                               _ss;
    size_t                                          _uri_len;
    size_t                                          _header_len;
    size_t                                          _body_len;
    size_t                                          _chunk_len;
    bool                                            _body_flag;
    bool                                            _chunked_transfer_flag;
    size_t                                          _client_max_body_size;
    std::vector<ServerBlock>                        _server_blocks;
    ServerBlock*                                    _server;
    Socket*                                         _socket;

// Private Member functions
    void                                        _findServerBlock(std::string host);
    void                                        _findDefaultServerBlock();

public:
// Constructor
    Request();

// Copy Constructor
    Request(const Request &rhs);

// Deconstructor
    ~Request();

// Getters
    int                                         getError() const;
    int                                         getVersionMajor() const;
    int                                         getVersionMinor() const;
    ParsingState                                getParsingState() const;
    HttpMethod                                  getMethod() const;
    ServerBlock*                                getServerBlock() const;
    Socket*                                     getSocket() const;
    const std::string&                          getMethodStr() const;
    const std::string&                          getPath() const;
    const std::string&                          getQuery() const;
    const std::string&                          getFragment() const;
    std::string&                                getBody();
    const std::map<std::string, std::string>&   getHeaders() const;

// Setters
    void                                        setServerBlocks(std::vector<ServerBlock> &server_blocks);
    void                                        setServerBlock(ServerBlock *server_block);
    void                                        setSocket(Socket* socket);

// Member functions
    void                                        parse(uint8_t *data, size_t size);
    void                                        clear();

};
