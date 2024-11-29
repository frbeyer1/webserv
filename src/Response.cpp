#include "../inc/Response.hpp"

// =============   Constructor   ============= //
Response::Response()
{
    _contentLength = 0;
    _code = 0;
}

// ============   Deconstructor   ============ //
Response::~Response(){}

// ================   Utils   ================ //


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

size_t Response::checkContent(){
    std::string tmp;
    std::ifstream file(_contentPath.c_str());//-----------------

    if (!file.is_open()) {
        std::cerr << "Error: Could not open the file." << std::endl;
        return (404);
    }
    while (std::getline(file, tmp)) {
        _content.append(tmp);
    }
    file.close();
    _contentLength = _content.length();
    return(200);
}

std::string Response::getTimeAndDate(){
    std::ostringstream  oss;
    time_t now = time(0);
    tm* localTime = localtime(&now);
    tzset();
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S", localTime);
    oss << std::string(buffer) << " " << tzname[0] << std::endl;
    return oss.str();
}

// std::string Response::getType(HttpRequest &request){
//     for (std::map<std::string, std::string>::const_iterator it = request.getHeaders().begin(); it != request.getHeaders().end(); it++){
//         // std::cout << it->first << ":" << it->second << std::endl;
//         if(it->first == "Accept")
//         {
//             size_t i = it->second.find(",");
//             std::string type = it->second.substr(0,i);//if type invalid or missing.. return _code
//             // std::cout << "type: " << type << std::endl;
//             return(type);
//         }
//     }
//     return(DEFAULT_type);//--
// }

std::string &Response::getResponseStr()
{
    return (_response_str);
}

size_t Response::getCode(){
    return (_code);
}

static std::string getMimeType(const std::string& filename)
{
    size_t dotPos = filename.find_last_of(".");
    
    if (dotPos != std::string::npos)
    {
        std::map<std::string, std::string> mimeTypes;

        mimeTypes[".html"] = "text/html";
        mimeTypes[".htm"] = "text/html";
        mimeTypes[".css"] = "text/css";
        mimeTypes[".js"] = "application/javascript";
        mimeTypes[".png"] = "image/png";
        mimeTypes[".jpg"] = "image/jpeg";
        mimeTypes[".jpeg"] = "image/jpeg";
        mimeTypes[".gif"] = "image/gif";
        mimeTypes[".pdf"] = "application/pdf";
        mimeTypes[".txt"] = "text/plain";
        mimeTypes[".gif"] = "image/gif";
        mimeTypes[".ico"] = "image/x-icon";
        mimeTypes[".mp3"] = "audio/mpeg";
        mimeTypes[".mp4"] = "video/mp4";
        mimeTypes[".sh"] = "application/x-sh";
        std::string extension = filename.substr(dotPos);
        std::map<std::string, std::string>::iterator it = mimeTypes.find(extension);
        if (it != mimeTypes.end())
            return it->second;
    }
    return "application/octet-stream";
}

// ======   Private member functions   ======= //

// ERROR PAGE
std::string Response::_buildDefaultErrorPage(int error_code)
{
    std::ostringstream  oss;

    oss << "<!DOCTYPE html><html><head><title>Error</title></head><body><center><h1>Error ";
    oss << lookupErrorMessage(error_code); 
    oss << "</h1><p>" << (error_code);
    oss << ".</p></center></body></html>";
    return (oss.str());
}

        // for (std::map<std::string, location_t>::const_iterator it = server.getLocations().begin(); it != server.getLocations().end(); it++){
        //     std::cout << "Key: " << it->first << std::endl;
        //     // std::cout << "  allowed_methods: " << it->second.allowed_methods << std::endl;
        //     std::cout << "  redirection: " << it->second.redirection << std::endl;
        //     std::cout << "  alias: " << it->second.alias << std::endl;
        //     std::cout << "  index: " << it->second.index << std::endl;
        //     std::cout << "  upload: " << it->second.upload << std::endl;
        //     std::cout << "  autoindex: " << (it->second.autoindex ? "true" : "false") << std::endl;
        //     std::cout << std::endl;
        // }
// GET
std::string Response::_GETmethod(HttpRequest &request, Server &server)
{
    std::ostringstream  oss;

    if(request.getPath() == "/")
    {
        if (!server.getLocations().begin()->second.index.empty())
            _contentPath =  server.getLocations().begin()->second.index;
        else if(server.getLocations().begin()->second.autoindex == true)
            _contentPath = server.getRoot() + "autoindex.html"; //check autoindex format?
        else
            return(_buildDefaultErrorPage(404));
    }
    // else if(request.getPath().find("/cgi-bin") != std::string::npos)
        // CgiHandler   cgi(request, server);
        // size_t       i = request.getPath().find("/cgi-bin/");
        // std::string  cgi_program = request.getPath().substr(i,npos);
        // allowed methods -> check in cgi
    else if (!server.getRoot().empty() && !request.getPath().empty())
        _contentPath = server.getRoot() + request.getPath();
    else
        return(_buildDefaultErrorPage(404));
    _contentType = getMimeType(_contentPath);
    _code = checkContent();
    if(_code != 200)
        return(_buildDefaultErrorPage(_code));
    oss << "HTTP/1.1 "<< _code <<" OK\r\n";
    oss << "Content-Type: "<< _contentType << "\r\n";
    oss << "Content-Length: "<< _contentLength << "\r\n";
    oss << "Date: "<< getTimeAndDate();
    oss << "\r\n";
    oss << _content;
    oss << "\r\n";
    return (oss.str());
}

// POST
std::string Response::_POSTmethod(HttpRequest &request, Server &server)
{
    std::ostringstream  oss;

    (void)server;
    (void)request;
    // allowed methods -> check in cgi
    // cgi
    // check if upload location exitsts and method allowed, <-defined in config
    // check if file to upload to exists
    // if file upload -> move to location 
    // if other create new file in location or if exist put data depending of type in there
    // how does it work when user wants access to file again
    // messages -> "successfull upload, saving, etc..."
    _contentPath = "docs/index.html";//-----------------
    checkContent();
    oss << "HTTP/1.1 "<< _code <<" OK\r\n";
    oss << "Content-Type: "<< _contentType << "\r\n";
    oss << "Content-Lenght: "<< _contentLength << "\r\n";
    oss << "\r\n";
    oss << _content;
    return (oss.str());
}

// DELETE
std::string Response::_DELETEmethod()
{
    std::ostringstream  oss;

    // allowed methods
    // cgi
    _contentPath = "docs/index.html";//-----------------
    checkContent();
    oss << "HTTP/1.1 "<< _code <<" OK\r\n";
    oss << "Content-Type: "<< _contentType << "\r\n";
    oss << "Content-Lenght: "<< _contentLength << "\r\n";
    oss << "\r\n";
    oss << _content;
    return (oss.str());
}

// ======   Public member functions   ======= //

void    Response::buildResponse(HttpRequest &request, Server &server){

    _code = request.getError();//<- valid?
    // _contentType = getType(request);
    if(_code != 200)
        _response_str = _buildDefaultErrorPage(_code);
    switch (request.getMethod())
    {
        case GET:
            _response_str = _GETmethod(request, server);
            break;
        case POST:
            _response_str = _POSTmethod(request, server);
            break;
        case DELETE:
            _response_str = _DELETEmethod();
            break;
        default:
                std::cout << "No method" << std::endl;
            break;
    };
};
