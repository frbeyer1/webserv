#include "../inc/Request.hpp"

// =============   Constructor   ============= //
Request::Request()
{
    clear();
    _socket = NULL;
}

// ===========   Copy Constructor   ========== //
Request::Request(const Request &rhs) : _ss(rhs._ss.str())
{
    if (this != &rhs)
	{
		_state = rhs._state;
        _error = rhs._error;
        _method = rhs._method;
        _path = rhs._path;
        _query = rhs._query;
        _fragment = rhs._fragment;
        _version_major = rhs._version_major;
        _version_minor = rhs._version_minor;
        _headers = rhs._headers;
        _body = rhs._body;
        _method_str = rhs._method_str;
        _header_field_name = rhs._header_field_name;
        _header_field_value = rhs._header_field_value;
        _chunk_length_str = rhs._chunk_length_str;
        _uri_len = rhs._uri_len;
        _header_len = rhs._header_len;
        _body_len = rhs._body_len;
        _chunk_len = rhs._chunk_len;
        _body_flag = rhs._body_flag;
        _chunked_transfer_flag = rhs._chunked_transfer_flag;
        _client_max_body_size = rhs._client_max_body_size;
        _server_blocks = rhs._server_blocks;
        _server = rhs._server;
        _socket = rhs._socket;
	}
	return ;
}

// ============   Deconstructor   ============ //
Request::~Request()
{
}

// ==============   Setters   ================ //
void    Request::setServerBlocks(std::vector<ServerBlock> &server_blocks)
{
    _server_blocks = server_blocks;
}

void    Request::setSocket(Socket *socket)
{
    _socket = socket;
}

// ==============   Getters   ================ //
int Request::getError() const
{
    return _error;
}

int Request::getVersionMajor() const
{
    return _version_major;
}
int Request::getVersionMinor() const
{
    return _version_minor;
}

HttpMethod    Request::getMethod() const
{
    return _method;
}

ParsingState    Request::getParsingState() const
{
    return _state;
}

ServerBlock*  Request::getServerBlock() const
{
    return _server;
}

const std::string   &Request::getMethodStr() const
{
    return _method_str;
}

const std::string   &Request::getPath() const
{
    return _path;
}

const std::string   &Request::getQuery() const
{
    return _query;
}

const std::string   &Request::getFragment() const
{
    return _fragment;
}

const std::string   &Request::getBody() const
{
    return _body;
}

const std::map<std::string, std::string>    &Request::getHeaders() const
{
    return _headers;
}

// ================   Utils   ================ //
/*
Checks if character is allowed to be in a URI
Characters allowed as specifed in the RFC:
    Alphanumeric: A-Z a-z 0-9
    Unreserved: - _ . ~
    Reserved:  * ' ( ) ; : @ & = + $ , / ? % # [ ]
*/
static bool    allowedURIChar(uint8_t c)
{
    if (c == '!' || (c >= '#' && c <= ';') || c == '=' || (c >= '?' && c <= '[') || c == ']' || c == '_' || (c >= 'a' && c <= 'z') || c == '~')
        return true;
    return false;
}

/* 
Checks if character is a valid token character and therefore allowed to be in a FieldName
Characters allowed as specifed in the RFC:
    A-Z a-z 0-9
    ! # $ % & ' * +  -  .  ^  _  `  |  ~               
*/
static bool    allowedFieldNameChar(uint8_t c)
{
    if ( c == '!' || (c >= '#' && c <= '\'') || c == '*' || c == '+' || c == '-' || c == '.' || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= '^' && c <= 'z') || c == '|' || c == '~')
        return true;
    return false;
}

/* 
Checks if character is allowed to be in a FieldValue
Characters allowed as specifed in the RFC:
    VCHAR (visible char) SP HTAB            
*/
static bool    allowedFieldValueChar(uint8_t c)
{
    if (c == '\t' || (c >= ' ' && c <= '~'))
        return true;
    return false;
}

/*
Checks and returns true if the URI path goes under the root directory
*/
static bool    checkPathUnderRoot(std::string path)
{
    char    *directory = strtok((char*)path.c_str(), "/");
    int     pos = 0;
 
    while (directory != NULL)
    {
        if (!strcmp(directory, ".."))
            pos--;
        else
            pos++;
        if (pos < 0)
            return true;
        directory = strtok(NULL, "/");
    }
    return false;
}

/*
does percent decoding on the URI
*/
static void percent_decoding(std::string &path)
{
    std::string decoded_path;

    for (size_t i = 0; i < path.length(); ++i)
    {
        if (path[i] == '%') 
        {
            if (i + 2 < path.length())
            {
                std::string hex = path.substr(i + 1, 2);
                char decoded_char = static_cast<char>(strtol(hex.c_str(), NULL, 16));
                decoded_path.push_back(decoded_char);
                i += 2;
            }
            else 
                decoded_path.push_back(path[i]);
        } 
        else 
            decoded_path.push_back(path[i]);
    }
    path = decoded_path;
}

/*
Checks for consecutive slashes in the path string and deletes them
*/
static void    checkPathConsecutiveSlashes(std::string &path)
{
    bool    slash_flag = false;

    for (size_t i = 0; i < path.size(); i++)
    {
        if (slash_flag && path[i] == '/')
        {
            path.erase(i, 1);
            i--;
        }
        if (path[i] == '/')
            slash_flag = true;
        else
            slash_flag = false; 
    }
}

/*
Deletes optional leading whitespace and optional trailing whitespace of the field line value
*/
static void    trimFieldValueStr(std::string &string)
{
    while (iswspace(string[0]))
        string.erase(0, 1);
    while (iswspace(string[string.size() - 1]))
        string.erase(string.size() - 1);
    return;
}

// ======   Private Member functions   ======= //
/*
finding the correct server for the request
*/
void    Request::_findServerBlock(std::string host)
{
    // search for server_block with the host header
    for (size_t i = 0; i < _server_blocks.size(); i++)
    {
        const std::vector<std::string> &server_names = _server_blocks[i]._server_names;
        for (size_t j = 0; j < server_names.size(); j++)
        {
            if (server_names[j] == host)
            {
                _server = &_server_blocks[i];
                _client_max_body_size = _server_blocks[i]._client_max_body_size;
                return;
            }
        }
    }
    // search for default server block
    for (size_t i = 0; i < _server_blocks.size(); i++)
    {
        if (_server_blocks[i]._host == _socket->getHost() && _server_blocks[i]._port == _socket->getPort())
        {
            _server = &_server_blocks[i];
            _client_max_body_size = _server_blocks[i]._client_max_body_size;
            return;
        }
    }
    if (_server == NULL)
        _error = BAD_REQUEST;
}

// ==========   Member functions   =========== //
/*
clears and resets all variables (except _client_max_body_size) in the object
*/
void    Request::clear()
{
    _state = Empty_Line;
    _error = OK;
    _method = NONE;
    _path = "";
    _query = "";
    _fragment = "";
    _version_major = 0;
    _version_minor = 0;
    _body = "";
    _method_str = "";
    _header_field_name = "";
    _header_field_value = "";
    _chunk_length_str = "";
    _uri_len = 0;
    _header_len = 0;
    _body_len = 0;
    _chunk_len = 0;
    _body_flag = false;
    _chunked_transfer_flag = false;
    _client_max_body_size = 0;
    _server = NULL;
    _ss.str("");
    _ss.clear();
    _headers.clear();
}

/*
partial parses the http Request octet by octet
*/
void    Request::parse(uint8_t *data, size_t size)
{
    uint8_t ch;

    if (_error != OK)
        return;
    for (size_t i = 0; i < size; i++) 
    {
        ch = data[i];
        switch (_state) {

        case Empty_Line:
            if (ch == CR || ch == LF)
                break;
            _method_str.push_back(ch);
            _state = Request_Line_Method;
            _state = Request_Line_Method;
            break;
        case Request_Line_Method:
            if (ch == ' ')
            {
                if(_method_str.compare("GET") == 0)
                    _method = GET;
                else if (_method_str.compare("POST") == 0)
                    _method = POST;
                else if (_method_str.compare("DELETE") == 0)
                    _method = DELETE;
                else
                    _method = NONE;
                _state = Request_Line_URI_Slash;
            }
            else
                _method_str.push_back(ch);
            if ((ch == ' ' && _method == NONE) || _method_str.size() > 7)
            {
                _error = NOT_IMPLEMENTED;
                return;
            }
            break;
        case Request_Line_URI_Slash:
            if (ch != '/')
            {
                _error = BAD_REQUEST;
                return;
            }
            _path.push_back(ch);
            _uri_len++;
            _state = Request_Line_URI_Path;
            break;
        case Request_Line_URI_Path:
            if (ch == '?')
            {
                _state = Request_Line_URI_Query;
                break;
            } 
            else if (ch == '#')
            {
                _state = Request_Line_URI_Fragment;
                break;
            }
            else if (ch == ' ')
            {
                _state = Request_Line_H;
                break;
            }
            else
            {
                if (!allowedURIChar(ch))
                {
                    _error = BAD_REQUEST;
                    return;
                }
                else if (_uri_len > MAX_URI_LENGTH)
                {
                    _error = URI_TOO_LONG;
                    return;
                }
                _path.push_back(ch);
                _uri_len++;
            }
            break;
        case Request_Line_URI_Query:
            if (ch == '#')
            {
                _state = Request_Line_URI_Fragment;
                break;
            }    
            else if (ch == ' ')
            {
                _state = Request_Line_H;
                break;
            }
            else
            {
                if (!allowedURIChar(ch))
                {
                    _error = BAD_REQUEST;
                    return;
                }
                else if (_uri_len > MAX_URI_LENGTH)
                {
                    _error = URI_TOO_LONG;
                    return;
                }
                _query.push_back(ch);
                _uri_len++;
            }
            break;
        case Request_Line_URI_Fragment:
            if (ch == ' ')
            {
               _state = Request_Line_H;
                break;
            }
            else
            {
                if (!allowedURIChar(ch))
                {
                    _error = BAD_REQUEST;
                    return;
                }
                else if (_uri_len > MAX_URI_LENGTH)
                {
                    _error = URI_TOO_LONG;
                    return;
                }
                _fragment.push_back(ch);
                _uri_len++;
            }
            break;
        case Request_Line_H:
            checkPathConsecutiveSlashes(_path);
            percent_decoding(_path);
            if (checkPathUnderRoot(_path))
            {
                _error = FORBIDDEN;
                return;
            }
            if (ch != 'H')
            {
                _error = BAD_REQUEST;
                return;
            }
            _state = Request_Line_HT;
            break;
        case Request_Line_HT:
            if (ch != 'T')
            {
                _error = BAD_REQUEST;
                return;
            }
            _state = Request_Line_HTT;
            break;
        case Request_Line_HTT:
            if (ch != 'T')
            {
                _error = BAD_REQUEST;
                return;
            }
            _state = Request_Line_HTTP;
            break;
        case Request_Line_HTTP:
            if (ch != 'P')
            {
                _error = BAD_REQUEST;
                return;
            }
            _state = Request_Line_HTTP_Slash;
            break;
        case Request_Line_HTTP_Slash:
            if (ch != '/')
            {
                _error = BAD_REQUEST;
                return;
            }
            _state = Request_Line_Version_Major;
            break;
        case Request_Line_Version_Major:
            if (!isdigit(ch))
            {
                _error = BAD_REQUEST;
                return;
            }
            _version_major = ch - '0';
            _state = Request_Line_Version_Dot;
            break;
        case Request_Line_Version_Dot:
            if (ch != '.')
            {
                _error = BAD_REQUEST;
                return;
            }
            _state = Request_Line_Version_Minor;
            break;
        case Request_Line_Version_Minor:
            if (!isdigit(ch))
            {
                _error = BAD_REQUEST;
                return;
            }
            _version_minor = ch - '0';
            _state = Request_Line_CR;
            break;
        case Request_Line_CR:
            if (ch != CR)
            {
                _error = BAD_REQUEST;
                return;
            }
            _state = Request_Line_LF;
            break;
        case Request_Line_LF:
            if (ch != LF)
            {
                _error = BAD_REQUEST;
                return;
            }
            _state = Header_Field_Start;
            break;
        case Header_Field_Start:
            if (ch == '\r')
            {
                _state = Header_Field_Blank_Line;
                break;
            }
            if (!allowedFieldNameChar(ch))
            {
                _error = BAD_REQUEST;
                return;
            }
            _header_field_name.push_back(ch);
            _header_len++;
            if (_header_len > MAX_HEADER_LENGTH)
            {
                _error = REQUEST_HEADER_FIELDS_TOO_LARGE;
                return;
            }
            _state = Header_Field_Name;
            break;
        case Header_Field_Name:
            if (ch == ':')
            {
                _state = Header_Field_Value;
                break;
            }
            if (!allowedFieldNameChar(ch))
            {
                _error = BAD_REQUEST;
                return;
            }
            _header_field_name.push_back(ch);
            _header_len++;
            if (_header_len > MAX_HEADER_LENGTH)
            {
                _error = REQUEST_HEADER_FIELDS_TOO_LARGE;
                return;
            }
            break;
        case Header_Field_Value:
            if (ch == CR)
            {
                if (_header_field_value.size() < 1)
                {
                    _error = BAD_REQUEST;
                    return;
                }
                _state = Header_Field_End;
                break;
            }
            if (!allowedFieldValueChar(ch))
            {
                _error = BAD_REQUEST;
                return;
            }
            _header_field_value.push_back(ch);
            _header_len++;
            if (_header_len > MAX_HEADER_LENGTH)
            {
                _error = REQUEST_HEADER_FIELDS_TOO_LARGE;
                return;
            }
            break;
        case Header_Field_End:
            if (ch != LF)
            {
                _error = BAD_REQUEST;
                return;
            }
            trimFieldValueStr(_header_field_value);
            if (_headers.count(_header_field_name))
            {
                _headers[_header_field_name] += ", ";
                _headers[_header_field_name] += _header_field_value;
            }
            else
                _headers.insert(std::pair<std::string, std::string>(_header_field_name, _header_field_value));
            _header_field_name.clear();
            _header_field_value.clear();
            _state = Header_Field_Start;
            break;
        case Header_Field_Blank_Line:
            if (ch != LF)
            {
                _error = BAD_REQUEST;
                return;
            }
            _state = Parsing_Finished;
            if (!_headers.count("Host"))
	        {
		        _error = BAD_REQUEST;
                return;
	        }
            else
            {
                _findServerBlock(_headers.find("Host")->second);
                if (_error != OK)
                return ;
            }
            if (_headers.count("Transfer-Encoding"))
            {
                if((_version_major == 1 && _version_minor == 0) || _version_major == 0)
                {
                    _error = BAD_REQUEST;
                    return;
                }
                if (_headers["Transfer-Encoding"] == "chunked")
                {
                    _body_flag = true;
                    _chunked_transfer_flag = true;
                    _state = Chunk_Length;
                }
                else
                {
                    _error = NOT_IMPLEMENTED;
                    return;
                }
            }
            if (_headers.count("Content-Length"))
            {
                if (_chunked_transfer_flag == true)
                {
                    _error = BAD_REQUEST;
                    return;
                }
                _body_len = atoi(_headers["Content-Length"].c_str());
                if (_body_len <= 0)
                {
                    _error = BAD_REQUEST;
                    return;
                }
                if (_body_len > _client_max_body_size)
                {
                    _error = PAYLOAD_TOO_LARGE;
                    return;
                }
                else
                {
                    _body_flag = true;
                    _state = Message_Body;
                }   
            }
            else if (_method == POST)
            {
                _error = LENGTH_REQUIRED;
                return ;
            }
            break;
        case Chunk_Length:
            _body_len++;
            if (ch == CR)
                _state = Chunk_Length_End;
            else if (ch == ';')
                _state = Chunk_Extensions;
            else if (isxdigit(ch))
                _chunk_length_str.push_back(ch);
            else
            {
                _error = BAD_REQUEST;
                return;
            }
            if (_body_len > _client_max_body_size)
            {
                _error = PAYLOAD_TOO_LARGE;
                return;
            }
            break;
        case Chunk_Extensions:
            // ignores the chunk extensions
            _body_len++;
            if (ch == CR)
                _state = Chunk_Length_End;
            if (_body_len > _client_max_body_size)
            {
                _error = PAYLOAD_TOO_LARGE;
                return;
            }
            break;
        case Chunk_Length_End:
            _body_len++;
            if (ch != '\n')
            {
                _error = BAD_REQUEST;
                return;
            }
            if (_body_len > _client_max_body_size)
            {
                _error = PAYLOAD_TOO_LARGE;
                return;
            }
            _state = Chunk_Data;
            _ss.str("");
            _ss.clear();
            _ss << _chunk_length_str;
            _ss >> std::hex >> _chunk_len;
            if (_chunk_len == 0)
                _state = Chunk_Last_CR;
            break;
        case Chunk_Data:
            if (_chunk_len > 0)
            {
                _body.push_back(ch);
                _body_len++;
                _chunk_len--;
            }
            if (_body_len > _client_max_body_size)
            {
                _error = PAYLOAD_TOO_LARGE;
                return;
            }
            if (_chunk_len == 0)
                _state = Chunk_Data_CR;
            break;
        case Chunk_Data_CR:
            if (ch != CR)
            {
                _error = BAD_REQUEST;
                return;
            }
            if (_body_len > _client_max_body_size)
            {
                _error = PAYLOAD_TOO_LARGE;
                return;
            }
            _body_len++;
            _state = Chunk_Data_LF;
            break;
        case Chunk_Data_LF:
            if (ch != LF)
            {
                _error = BAD_REQUEST;
                return;
            }
            if (_body_len > _client_max_body_size)
            {
                _error = PAYLOAD_TOO_LARGE;
                return;
            }
            _state = Chunk_Length;
            _body_len++;
            break;
        case Chunk_Last_CR:
            if (ch != CR)
            {
                _error = BAD_REQUEST;
                return;
            }
            if (_body_len > _client_max_body_size)
            {
                _error = PAYLOAD_TOO_LARGE;
                return;
            }
            _state = Chunk_Last_LF;
            _body_len++;
            break;
        case Chunk_Last_LF:
            if (ch != LF)
            {
                _error = BAD_REQUEST;
                return;
            }
            if (_body_len > _client_max_body_size)
            {
                _error = PAYLOAD_TOO_LARGE;
                return;
            }
            _state = Chunk_Trailer_Section;
            _body_len++;
            break;
        case Chunk_Trailer_Section:
            // ignores the chunk trailer section
            _state = Parsing_Finished;
            break;
        case Message_Body:
            if (_body_len)
            {
                _body.push_back(ch);
                _body_len--;
            }
            if (_body_len == 0)
                _state = Parsing_Finished;
            break;
        case Parsing_Finished:
            return;
        }
    }
}
