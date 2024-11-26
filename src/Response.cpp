#include "../inc/Response.hpp"

// =============   Constructor   ============= //
Response::Response()
{
    _file = "";
    _content = "";
    _contentType = "";
    _contentLenght = 0;
    _code = 0;
}

// ============   Deconstructor   ============ //
Response::~Response(){}

// ================   Utils   ================ //
/*
return an description of the error_code as a string
*/

static  std::string lookupErrorMessage(int error_code)
{
    switch (error_code) {

    case  400: return "Bad Request";
    case  401: return "Unauthorized";
    case  402: return "Payment Required";
    case  403: return "Forbidden";
    case  404: return "Not Found";
    case  405: return "Method Not Allowed";
    case  406: return "Not Acceptable";
    case  407: return "Proxy Authentication Required";
    case  408: return "Request Timeout";
    case  409: return "Conflict";
    case  410: return "Gone";
    case  411: return "Length Required";
    case  412: return "Precondition Failed";
    case  413: return "Payload Too Large";
    case  414: return "URI Too Long";
    case  415: return "Unsupported Media Type";
    case  416: return "Range Not Satisfiable";
    case  417: return "Expectation Failed";
    case  426: return "Upgrade Required";
    case  428: return "Precondition Required";
    case  429: return "Too Many Requests";
    case  431: return "Request Header Fields Too Large";
    case  451: return "Unavailable For Legal Reasons";
    case  500: return "Internal Server Error";
    case  501: return "Not Implemented";
    case  502: return "Bad Gateway";
    case  503: return "Service Unavailable";
    case  504: return "Gateway Timeout";
    case  505: return "HTTP Version Not Supported";
    case  511: return "Network Authentication Required";
    default: return "Undefined";

    }
}

// ======   Private member functions   ======= //
/*
builds an Default error page and returns it as string
*/
std::string Response::_buildDefaultErrorPage(int error_code)
{
    std::ostringstream  oss; //_file

    oss << "<!DOCTYPE html><html><head><title>Error</title></head><body><center><h1>Error ";
    oss << lookupErrorMessage(error_code); 
    oss << "</h1><p>" << (error_code);
    oss << ".</p></center></body></html>";
    return (oss.str());
}
// GET
std::string Response::_GETResponse()
{
    std::ostringstream  oss; //_file

    // check max bodysize of client
    // check if get is allowed??
    oss << "HTTP?1.1 "<< _code <<" OK\r\n";
    oss << "Content-Type: "<< _contentType << "\r\n";
    oss << "Content-Lenght: "<< _contentLenght << "\r\n";
    oss << "\r\n";
    // oss << _content;
    return (oss.str());
}

// std::string Response::_buildContent(){
//     getCode();
//     _buildDefaultErrorPage(_code);
//     if
// };
// POST
// DELETE

// ======   Public member functions   ======= //
size_t Response::getCode(){
    return(200);
}

std::string Response::getType(){
    return("test/html");
}
void    Response::build(HttpRequest &request){

    // getTargetFile();
    _code = getCode();
    _contentType = getType();

    if(_code != 200)
        _buildDefaultErrorPage(_code);

    switch (request.getMethod())
    {
    case GET:
        _response_str = _GETResponse();
        //  _GETResponse(_file);
        break;
    
    default:
        break;
    }
};

std::string &Response::getResponseStr()
{
    return (_response_str);
}