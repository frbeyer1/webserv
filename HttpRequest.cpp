#include "HttpRequest.hpp"

// Default Constructor
HttpRequest::HttpRequest()
{
    _state = Method;
    _error = 0;
    _method = NONE;
    _path = "";
    _query = "";
    _fragment = "";
    _version_major = 0;
    _version_minor = 0;
    _header_field_name = "";
    _header_field_value = "";
    _method_str = "";
    _method_str_len = 0;
    _uri_len = 0;
    _body_lenght = 0;
    _body_flag = false;
    _chuncked_transfer_flag = false;
}

// Deconstructor
HttpRequest::~HttpRequest()
{

}

/* 
Checks if character is allowed to be in a URI
Characters allowed as specifed in the RFC:
    Alphanumeric: A-Z a-z 0-9
    Unreserved: - _ . ~
    Reserved:  * ' ( ) ; : @ & = + $ , / ? % # [ ]
*/
bool    allowedURIChar(char c)
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
bool    allowedFieldNameChar(char c)
{
    if ( c == '!' || c >= '#' && c <= '\'' || c == '*' || c == '+' || c == '-' || c == '.'
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

void    HttpRequest::parse(char *data, size_t size)
{
    char    character;

    if (_error)
        return;
    for (size_t i = 0; i < size; i++)
    {
        character = data[i];
        switch (_state)
        {
            case Method:
                _method_str.push_back(character);
                _method_str_len++;
                if (_method_str.compare(0, _method_str_len, "GET") == 0)
                    _method = GET;
                else if (_method_str.compare(0, _method_str_len, "POST") == 0)
                    _method = GET;
                else if (_method_str.compare(0, _method_str_len, "DELETE") == 0)
                    _method = GET;
                else
                {
                    _method = NONE;
                    _error = 501;
                    // error msg
                    return;
                }
                if (_method == GET && _method_str_len == 3 ||
                    _method == POST && _method_str_len == 4 ||
                    _method == DELETE && _method_str_len == 6)
                    _state = Space;
                break;
            case Space:
                if (character != ' ')
                {
                    _error = 400;
                    // error msg
                    return;
                }
                _state = URI_Path_Slash;
                break;
            case URI_Path_Slash:
                if (character != '/')
                {
                    _error = 400;
                    // error msg
                    return;
                }
                _path.push_back(character);
                _uri_len++;
                _state = URI_Path;
                break;
            case URI_Path:
                _uri_len++;
                if (character == '?')
                {
                    _state = URI_Query;
                    return;
                }
                if (character == '#')
                {
                    _state = URI_Fragment;
                    return;
                }
                if (character == ' ')
                {
                    _state = Version_H;
                    return;
                }
                if (character == '/')
                {
                    _error = 400;
                    // error msg
                    return;
                }
                if (!allowedURIChar(character))
                {
                    _error = 400;
                    // error msg
                    return;
                }
                if (_uri_len > MAX_URI_LENGHT)
                {
                    _error = 414;
                    // error msg
                    return;
                }
                _path.push_back(character);
                break;
            case URI_Query:
                _uri_len++;
                if (character == '#')
                {
                    _state = URI_Fragment;
                    return;
                }
                if (character == ' ')
                {
                    _state = Version_H;
                    return;
                }
                if (!allowedURIChar(character))
                {
                    _error = 400;
                    // error msg
                    return;
                }
                if (_uri_len > MAX_URI_LENGHT)
                {
                    _error = 414;
                    // error msg
                    return;
                }
                _query.push_back(character);
                break;
            case URI_Fragment:
                _uri_len++;
                if (character == ' ')
                {
                    _state = Version_H;
                    return;
                }
                if (!allowedURIChar(character))
                {
                    _error = 400;
                    // error msg
                    return;
                }
                if (_uri_len > MAX_URI_LENGHT)
                {
                    _error = 414;
                    // error msg
                    return;
                }
                _fragment.push_back(character);
                break;
            case Version_H:
                if (checkPathUnderRoot(_path))
                {
                    _error = 403;
                    // error msg
                    return;
                }
                if (character != 'H')
                {
                    _error = 400;
                    // error msg
                    return;
                }
                _state = Version_HT;
                break;
            case Version_HT:
                if (character != 'T')
                {
                    _error = 400;
                    // error msg
                    return;
                }
                _state = Version_HTT;
                break;
            case Version_HTT:
                if (character != 'T')
                {
                    _error = 400;
                    // error msg
                    return;
                }
                _state = Version_HTTP;
                break;
            case Version_HTTP:
                if (character != 'P')
                {
                    _error = 400;
                    // error msg
                    return;
                }
                _state = Version_HTTP_Slash;
                break;
            case Version_HTTP_Slash:
                if (character != '/')
                {
                    _error = 400;
                    // error msg
                    return;
                }
                _state = Version_Major;
                break;
            case Version_Major:
                if (!isdigit(character))
                {
                    _error = 400;
                    // error msg
                    return;
                }
                _version_major = character - '0';
                _state = Version_Dot;
                break;
            case Version_Dot:
                if (character != '.')
                {
                    _error = 400;
                    // error msg
                    return;
                }
                _state = Version_Minor;
                break;
            case Version_Minor:
                if (!isdigit(character))
                {
                    _error = 400;
                    // error msg
                    return;
                }
                _version_minor = character - '0';
                _state = Request_Line_CR;
                break;
            case Request_Line_CR:
                if (character != '\r')
                {
                    _error = 400;
                    // error msg
                    return;
                }
                _state = Request_Line_LF;
                break;
            case Request_Line_LF:
                if (character != '\n')
                {
                    _error = 400;
                    // error msg
                    return;
                }
                _state = Field_Start;
                break;
            case Field_Start:
                if (character == '\r')
                {
                    _state = Field_Blank_Line;
                    return;
                }
                if (!allowedFieldNameChar(character))
                {
                    _error = 400;
                    // error msg
                    return;
                }
                _header_field_name.push_back(character);
                _state = Field_Name;
                break;
            case Field_Name:
                if (!allowedFieldNameChar(character))
                {
                    _error = 400;
                    // error msg
                    return;
                }
                if (character == ':')
                {
                    _state = Field_Value;
                    return;
                }
                _header_field_name.push_back(character);
                break;
            case Field_Value:
                if (character == '\r')
                {
                    _state = Field_End;
                    return;
                }
                _header_field_value.push_back(character);  
                break;
            case Field_End:
                // set _body_flag and _chuncked_transfer_flag and content-lenght
                if (character != '\n')
                {
                    _error = 400;
                    // error msg
                    return;
                }
                trimFieldValueStr(_header_field_value);
                _it = _fields.find(_header_field_name);
                if (_it != _fields.end())
                {
                    _it->second += ", ";
                    _it->second += _header_field_value;
                }
                else
                    _fields.insert(std::pair<std::string, std::string>(_header_field_name, _header_field_value));
                _header_field_name.clear();
                _header_field_value.clear();
                _state = Field_Start;
                break;
            case Field_Blank_Line:
                if (character != '\n')
                {
                    _error = 400;
                    // error msg
                    return;
                }
                if (_body_flag)
                {
                    if (_chuncked_transfer_flag)
                        _state = Chuncked_Lenght;
                    _state = Message_Body;
                }
                else
                    _state = Parsing_Finished;
                break;
            // case Chuncked_Lenght:
            //     break;
            case Message_Body:
                if (_body_lenght)
                {
                    _body.push_back(character);
                    _body_lenght--;
                }
                if (_body_lenght == 0)
                {
                    _state = Parsing_Finished;
                    return ;
                } 
                break;
            case Parsing_Finished:
                break;
        }
    }
}

// parse the body

// check the lenght of the header fields and values and how many headers can be there and the lenght of the whole request
// check body size

// do Percent decoding (what happens if URI starts with %2F (/))

// httpRequest Deconstructor (copy assignment operator ???)


/*
Transfer-Encoding is an HTTP header used to specify the form of encoding used to safely transfer a payload body in HTTP messages. It is primarily used for chunked transfer encoding, which allows data to be sent in a series of chunks rather than as a single block.
Key Points about Transfer-Encoding:

    Purpose: The Transfer-Encoding header allows the sending of data in a way that can be processed as it arrives, rather than requiring the entire body to be present before processing begins. This can be useful for streaming data or large payloads where the sender may not know the total size of the payload in advance.

    Chunked Transfer Encoding:
        This is the most common form of Transfer-Encoding. When using chunked encoding, the body of the message is sent in a series of chunks. Each chunk has its own size defined in bytes, followed by a CRLF (Carriage Return and Line Feed), and the data itself, also followed by a CRLF.
        The end of the transmission is indicated by a chunk size of zero.

    Header Format: When an HTTP response uses Transfer-Encoding: chunked, it should not include a Content-Length header because the length of the data is not known at the beginning.
        Example:

         
        Transfer-Encoding: chunked

    Multiple Encodings: Although chunked is the most well-known, Transfer-Encoding can specify multiple encodings applied sequentially. For example, you might see:

     
    Transfer-Encoding: gzip, chunked

    Here, the data is first compressed using gzip and then sent as chunked.

    Client and Server Support: Both the client and server must support Transfer-Encoding to properly handle chunked responses. If a client does not understand chunked encoding, it may not process the response correctly.

    Use Cases:
        Streaming Large Files: Sending files in chunks helps mitigate memory usage on both client and server.
        Dynamic Content: For web applications generating content dynamically, chunked encoding allows the server to send parts of the response as they are created.

    HTTP/1.1 and Beyond: Transfer-Encoding is defined in HTTP/1.1 as part of the protocol; however, HTTP/2 and later versions handle streaming and framing in a different manner, making some aspects of Transfer-Encoding less relevant.

Example of Chunked Transfer-Encoding:

A typical HTTP response using chunked encoding might look as follows:

 
HTTP/1.1 200 OK
Transfer-Encoding: chunked
Content-Type: text/plain

4
Wiki
5
pedia
0

In this example, the body consists of two chunks: the first chunk of size 4 sends "Wiki," followed by the second chunk of size 5 sending "pedia." The 0 at the end indicates that there are no more chunks.
Conclusion

Transfer-Encoding is a useful feature in HTTP for streaming and sending data efficiently, especially when the size of the payload is not known beforehand. Its chunked encoding mechanism enhances the ability to handle large or dynamically generated responses effectively.
*/