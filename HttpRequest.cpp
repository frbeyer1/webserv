#include "HttpRequest.hpp"

// =============   Constructor   ============= //
HttpRequest::HttpRequest()
{
    _state = Empty_Line_CR;
    _error = 0;
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
    _chunk_lenght_str = "";
    _uri_len = 0;
    _header_len = 0;
    _body_len = 0;
    _chunk_len = 0;
    _body_flag = false;
    _chuncked_transfer_flag = false;
}

// ============   Deconstructor   ============ //
HttpRequest::~HttpRequest()
{
}

// ==============   Getters   ================ //
int HttpRequest::getError()
{
    return _error;
}

int HttpRequest::getVersionMajor()
{
    return _version_major;
}
int HttpRequest::getVersionMinor()
{
    return _version_minor;
}

HttpMethod    HttpRequest::getMethod()
{
    return  _method;
}

ParsingState    HttpRequest::getParsingState()
{
    return  _state;
}

const std::string   &HttpRequest::getPath()
{
    return _path;
}

const std::string   &HttpRequest::getQuery()
{
    return _query;
}

const std::string   &HttpRequest::getFragment()
{
    return _fragment;
}

const std::string   &HttpRequest::getBody()
{
    return _body;
}

const std::map<std::string, std::string>    &HttpRequest::getFields()
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
bool    allowedURIChar(uint8_t c)
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
bool    allowedFieldNameChar(uint8_t c)
{
    if ( c == '!' || (c >= '#' && c <= '\'') || c == '*' || c == '+' || c == '-' || c == '.'
        || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= '^' && c <= 'z') || c == '|' || c == '~')
        return true;
    return false;
}

/*
Checks and returns true if the URI path goes under the root directory
*/
bool    checkPathUnderRoot(std::string path)
{
    char    *directory = strtok((char*)path.c_str(), "/");
    int     pos = 0;
 
    while (directory != NULL)
    {
        if (strcmp(directory, ".."))
            pos--;
        else
            pos++;
        if (pos < 0)
            return true;
        directory = strtok(NULL, "-");
    }
    return false;
}

/*
Deletes optional leading whitespace and optional trailing whitespace of the field line value
*/
void    trimFieldValueStr(std::string &string)
{
    while (iswspace(string[0]))
        string.erase(0, 1);
    while (iswspace(string[string.size() - 1]))
        string.erase(string.size() - 1);
    return;
}


// ==========   Member functions   =========== //
/*
partial parses the http Request octet by octet
*/
void    HttpRequest::parse(char *data, size_t size)
{
    uint8_t character;

    if (_error)
        return;
    for (size_t i = 0; i < size; i++)
    {
        character = data[i];
        switch(_state) 
        {
            case Empty_Line_CR:
                if (character == '\r')
                {
                    _state = Empty_Line_LF;
                    continue;
                }
                _method_str.push_back(character);
                _state = Request_Line_Method;
                break;
            case Empty_Line_LF:
                if (character != '\n')
                {
                    _error = 400;
                    return;
                }
                _state = Request_Line_Method;
                break;
            case Request_Line_Method:
                _method_str.push_back(character);
                if(_method_str.compare(0, _method_str.size(), "GET", 0, _method_str.size()) == 0)
                    _method = GET;
                else if (_method_str.compare(0, _method_str.size(), "POST", 0, _method_str.size()) == 0)
                    _method = POST;
                else if (_method_str.compare(0, _method_str.size(), "DELETE", 0, _method_str.size()) == 0)
                    _method = DELETE;
                else
                {
                    _method = NONE;
                    _error = 501;
                    return;
                }
                if ((_method == GET && _method_str.size() == 3) ||
                    (_method == POST && _method_str.size() == 4) ||
                    (_method == DELETE && _method_str.size() == 6))
                    _state = Request_Line_Space;
                break;
            case Request_Line_Space:
                if (character != ' ')
                {
                    _error = 400;
                    continue;;
                }
                _state = Request_Line_URI_Slash;
                break;
            case Request_Line_URI_Slash:
                if (character != '/')
                {
                    _error = 400;
                    return;
                }
                _path.push_back(character);
                _uri_len++;
                _state = Request_Line_URI_Path;
                break;
            case Request_Line_URI_Path:
                if (character == '?')
                {
                    _state = Request_Line_URI_Query;
                    continue;
                }
                if (character == '#')
                {
                    _state = Request_Line_URI_Fragment;
                    continue;
                }
                if (character == ' ')
                {
                    _state = Request_Line_H;
                    continue;
                }
                if (!allowedURIChar(character))
                {
                    _error = 400;
                    return;
                }
                if (_uri_len > MAX_URI_LENGHT)
                {
                    _error = 414;
                    return;
                }
                _path.push_back(character);
                _uri_len++;
                break;
            case Request_Line_URI_Query:
                if (character == '#')
                {
                    _state = Request_Line_URI_Fragment;
                    continue;
                }
                if (character == ' ')
                {
                    _state = Request_Line_H;
                    continue;
                }
                if (!allowedURIChar(character))
                {
                    _error = 400;
                    return;
                }
                if (_uri_len > MAX_URI_LENGHT)
                {
                    _error = 414;
                    return;
                }
                _query.push_back(character);
                _uri_len++;
                break;
            case Request_Line_URI_Fragment:
                if (character == ' ')
                {
                    _state = Request_Line_H;
                    continue;
                }
                if (!allowedURIChar(character))
                {
                    _error = 400;
                    return;
                }
                if (_uri_len > MAX_URI_LENGHT)
                {
                    _error = 414;
                    return;
                }
                _fragment.push_back(character);
                _uri_len++;
                break;
            case Request_Line_H:
                if (checkPathUnderRoot(_path))
                {
                    _error = 403;
                    return;
                }
                if (character != 'H')
                {
                    _error = 400;
                    return;
                }
                _state = Request_Line_HT;
                break;
            case Request_Line_HT:
                 if (character != 'T')
                {
                    _error = 400;
                    return;
                }
                _state = Request_Line_HTT;
                break;
            case Request_Line_HTT:
                if (character != 'T')
                {
                    _error = 400;
                    return;
                }
                _state = Request_Line_HTTP;
                break;
            case Request_Line_HTTP:
                if (character != 'P')
                {
                    _error = 400;
                    return;
                }
                _state = Request_Line_HTTP_Slash;
                break;
            case Request_Line_HTTP_Slash:
                if (character != '/')
                {
                    _error = 400;
                    return;
                }
                _state = Request_Line_Version_Major;
                break;
            case Request_Line_Version_Major:
                if (!isdigit(character))
                {
                    _error = 400;
                    return;
                }
                _version_major = character - '0';
                _state = Request_Line_Version_Dot;
                break;
            case Request_Line_Version_Dot:
                if (character != '.')
                {
                    _error = 400;
                    return;
                }
                _state = Request_Line_Version_Minor;
                break;
            case Request_Line_Version_Minor:
                if (!isdigit(character))
                {
                    _error = 400;
                    return;
                }
                _version_minor = character - '0';
                _state = Request_Line_CR;
                break;
            case Request_Line_CR:
                if (character != '\r')
                {
                    _error = 400;
                    return;
                }
                _state = Request_Line_LF;
                break;
            case Request_Line_LF:
                if (character != '\n')
                {
                    _error = 400;
                    return;
                }
                _state = Header_Field_Start;
                break;
            case Header_Field_Start:
                if (character == '\r')
                {
                    _state = Header_Field_Blank_Line;
                    continue;
                }
                if (!allowedFieldNameChar(character))
                {
                    _error = 400;
                    return;
                }
                _header_field_name.push_back(character);
                _header_len++;
                if (_header_len > MAX_HEADER_LENGHT)
                {
                    _error = 431;
                    return;
                }
                _state = Header_Field_Name;
                break;
            case Header_Field_Name:
                if (!allowedFieldNameChar(character))
                {
                    _error = 400;
                    return;
                }
                if (character == ':')
                {
                    _state = Header_Field_Value;
                    return;
                }
                _header_field_name.push_back(character);
                _header_len++;
                if (_header_len > MAX_HEADER_LENGHT)
                {
                    _error = 431;
                    return;
                }
                break;
            case Header_Field_Value:
                if (character == '\r')
                {
                    _state = Header_Field_End;
                    return;
                }
                _header_field_value.push_back(character);
                _header_len++;
                if (_header_len > MAX_HEADER_LENGHT)
                {
                    _error = 431;
                    return;
                }
                break;
            case Header_Field_End:
                if (character != '\n')
                {
                    _error = 400;
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
                if (character != '\n')
                {
                    _error = 400;
                    return;
                }
                _state = Parsing_Finished;
                if (_headers.count("Transfer-Encoding"))
                {
                    if((_version_major == 1 && _version_minor == 0) || _version_major == 0)
                    {
                        _error = 400;
                        return;
                    }
                    if (_headers["Transfer-Encoding"] == "chunked")
                    {
                        _body_flag = true;
                        _chuncked_transfer_flag = true;
                        _state = Chunk_Lenght;
                    }
                    else
                    {
                        _error = 501;
                        return;
                    }
                }
                if (_headers.count("Content-Lenght"))
                {
                    if (_chuncked_transfer_flag == true)
                    {
                        _error = 400;
                        return;
                    }
                    _body_len = atoi(_headers["Content-Lenght"].c_str());
                    if (_body_len <= 0)
                    {
                        _error = 400;
                        return;
                    }
                    else
                    {
                        _body_flag = true;
                        _state = Message_Body;
                    }   
                }
                if (_body_len > MAX_BODY_SIZE)
                {
                    _error = 413;
                    return;
                }
                break;    
            case Chunk_Lenght:
                _body_len++;
                if (isxdigit(character))
                    _chunk_lenght_str.push_back(character);
                else if(character == '\r')
                    _state = Chunk_Lenght_LF;
                else if(character == ';')
                    _state = Chunk_Extensions;
                else
                {
                    _error = 400;
                    return;
                }
                if (_body_len > MAX_BODY_SIZE)
                {
                    _error = 413;
                    return;
                }
                break;
            case Chunk_Extensions:
                _body_len++;
                if (character == '\r')
                    _state = Chunk_Lenght_LF;
                if (_body_len > MAX_BODY_SIZE)
                {
                    _error = 413;
                    return;
                }
                break;
            case Chunk_Lenght_LF:
                if (character != '\n')
                {
                    _error = 400;
                    return;
                }
                _body_len++;
                if (_body_len > MAX_BODY_SIZE)
                {
                    _error = 413;
                    return;
                }
                _state = Chunk_Data;
                _ss.str("");
                _ss.clear();
                _ss << _chunk_lenght_str;
                _ss >> std::hex >> _chunk_len;
                if (_chunk_len == 0)
                    _state = Chunk_Last_CR;
                break;
            case Chunk_Data:
                if (_chunk_len > 0)
                {
                    _body.push_back(character);
                    _body_len++;
                    _chunk_len--;
                }
                if (_body_len > MAX_BODY_SIZE)
                {
                    _error = 413;
                    return;
                }
                if (_chunk_len == 0)
                    _state = Chunk_Data_CR;
                break;
            case Chunk_Data_CR:
                if (character != '\r')
                {
                    _error = 400;
                    return;
                }
                _state = Chunk_Data_LF;
                _body_len++;
                if (_body_len > MAX_BODY_SIZE)
                {
                    _error = 413;
                    return;
                }
                break;
            case Chunk_Data_LF:
                if (character != '\n')
                {
                    _error = 400;
                    return;
                }
                _state = Chunk_Lenght;
                _body_len++;
                if (_body_len > MAX_BODY_SIZE)
                {
                    _error = 413;
                    return;
                }
                break;
            case Chunk_Last_CR:
                if (character != '\r')
                {
                    _error = 400;
                    return;
                }
                _state = Chunk_Last_LF;
                _body_len++;
                if (_body_len > MAX_BODY_SIZE)
                {
                    _error = 413;
                    return;
                }
                break;
            case Chunk_Last_LF:
                if (character != '\n')
                {
                    _error = 400;
                    return;
                }
                _state = Chunk_Trailer_Section;
                _body_len++;
                if (_body_len > MAX_BODY_SIZE)
                {
                    _error = 413;
                    return;
                }
                break;
            case Chunk_Trailer_Section:
                _state = Parsing_Finished;
                break;
            case Message_Body:
                if (_body_len)
                {
                    _body.push_back(character);
                    _body_len--;
                }
                if (_body_len == 0)
                {
                    _state = Parsing_Finished;
                    continue ;
                } 
                break;
            case Parsing_Finished:
                return;
                break;
        }
    }
}

// ===============   To Do   ================= //

// do Chunk trailing section parsing ???

// do Percent decoding (what happens if URI starts with %2F (/)) ???

// check for double // in the path ???
