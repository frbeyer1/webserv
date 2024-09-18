#include "HttpRequest.hpp"

// Default Constructor
HttpRequest::HttpRequest()
{
    _state = Request_Line_Method;
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
bool    HttpRequest::allowedURIChar(char c)
{
    if (c == '!' || (c >= '#' && c <= ';') || c == '=' || (c >= '?' && c <= '[') || c == ']' || c == '_' || (c >= 'a' && c <= 'z') || c == '~')
        return true;
    return false;
}

void    HttpRequest::parse(char character)
{
    switch (_state)
    {
        case Request_Line_Method:
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
                _state = Request_Line_First_Space;
            break;
        case Request_Line_First_Space:
            if (character != ' ')
            {
                _error = 400;
                // error msg
                return;
            }
            _state = Request_Line_URI_Path_Slash;
            break;
        case Request_Line_URI_Path_Slash:
            if (character != '/')
            {
                _error = 400;
                // error msg
                return;
            }
            _path.push_back(character);
            _state = Request_Line_URI_Path;
            break;
        case Request_Line_URI_Path:
            if (character == '?')
            {
                _state = Request_Line_URI_Query;
                return;
            }
            if (character == '#')
            {
                _state = Request_Line_URI_Fragment;
                return;
            }
            if (character == ' ')
            {
                _state = Request_Line_Version_H;
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
            _path.push_back(character);
            break;
        case Request_Line_URI_Query:
            if (character == '#')
            {
                _state = Request_Line_URI_Fragment;
                return;
            }
            if (character == ' ')
            {
                _state = Request_Line_Version_H;
                return;
            }
            if (!allowedURIChar(character))
            {
                _error = 400;
                // error msg
                return;
            }
            _query.push_back(character);
            break;
        case Request_Line_URI_Fragment:
            if (character == ' ')
            {
                _state = Request_Line_Version_H;
                return;
            }
            if (!allowedURIChar(character))
            {
                _error = 400;
                // error msg
                return;
            }
            _fragment.push_back(character);
            break;
        case  Request_Line_Version_H:
            if (character != 'H')
            {
                _error = 400;
                // error msg
                return;
            }
            _state = Request_Line_Version_HT;
            break;
        case Request_Line_Version_HT:
            if (character != 'T')
            {
                _error = 400;
                // error msg
                return;
            }
            _state = Request_Line_Version_HTT;
            break;
        case Request_Line_Version_HTT:
            if (character != 'T')
            {
                _error = 400;
                // error msg
                return;
            }
            _state = Request_Line_Version_HTTP;
            break;
        case Request_Line_Version_HTTP:
            if (character != 'P')
            {
                _error = 400;
                // error msg
                return;
            }
            _state = Request_Line_Version_HTTP_Slash;
            break;
        case Request_Line_Version_HTTP_Slash:
            if (character != '/')
            {
                _error = 400;
                // error msg
                return;
            }
            _state = Request_Line_Version_Major;
            break;
        case Request_Line_Version_Major:
            if (!isdigit(character))
            {
                _error = 400;
                // error msg
                return;
            }
            _version_major = character - '0';
            _state = Request_Line_Version_Dot;
            break;
        case Request_Line_Version_Dot:
            if (character != '.')
            {
                _error = 400;
                // error msg
                return;
            }
            _state = Request_Line_Version_Minor;
            break;
        case Request_Line_Version_Minor:
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
            _state = Header_Field_Name_Start;
            break;
        case Header_Field_Name_Start:



            if (character == '\r')
            {
                _state = Header_Field_Blank_Line;
                return;
            }
            _state = Header_Field_Name;
            break;
        case Header_Field_Name:
            if (character == ':')
            {
                _state = Header_Field_Value;
                return;
            }
            _header_field_name.push_back(character);
            break;
        case Header_Field_Value:
            if (character == '\r')
            {
                _state = Header_Field_LF;
                return;
            }
            _header_field_value.push_back(character);
            break;
        case Header_Field_LF:
            if (character != '\n')
            {
                _error = 400;
                // error msg
                return;
            }
            _header_fields.insert(std::pair(_header_field_name, _header_field_value));
            _header_field_name.clear();
            _header_field_value.clear();
            _state = Header_Field_Name_Start;
            break;
        case Header_Field_Blank_Line:
            if (character != '\n')
            {
                _error = 400;
                // error msg
                return;
            }
            // _state = ;
            break;
        // parsing of the body
        // case ...:
            // if (_method == GET)
            // {
                // _state = Parsing_Finished;
                // return ;
            // }
        


        case Parsing_Finished:
            // nothing to do
            break;
    }
}

// check the header field map if the key is already there and add it to the vaulue with ", " (exception is Set-Cookie)
// check if the characters are valid inside the header fields
// check the lenght of the header fileds and how many headers can be there
// check the length of URI
// check body size
// check if URI Path start with .. => error (what happens if /folder1/../.. ???)
// httpRequest Deconstructor (copy assignment operator ???)
// do Percent Encoding
// make parser function able to work with char * (multiple chars)