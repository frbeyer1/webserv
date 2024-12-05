#include "../inc/Response.hpp"

// =============   Constructor   ============= //
Response::Response()
{
    _response = "";
    _error = OK;
    _connection = "";
    _content = "";
    _content_type = "";
    _date = "";
    _location = "";
}

// ============   Deconstructor   ============ //
Response::~Response()
{

}

// ==============   Getters   ================ //
const std::string &Response::getResponse() const
{
    return (_response);
}

int Response::getError() const
{
    return (_error);
}

const std::string &Response::getConnection() const
{
    return (_connection);
}

// ================   Utils   ================ //
/*
returns an string accordingly to the error_code
*/
static  std::string lookupErrorMessage(int error_code)
{
    switch (error_code) {
    case  200: return "OK";
    case  301: return "Moved Permanently";
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

/*
reads the file with the given path in binary mode and returns its content as a string
*/
static std::string    readFile(std::string path)
{
    std::ifstream   file(path.c_str(), std::ios::binary);
    std::string     content;

    if (!file.is_open())
    {
        Logger::log(RED, ERROR, "Failed opening the file: %s", path);
        exit (EXIT_FAILURE);
    }

    std::ostringstream oss;
    oss << file.rdbuf();
    content = oss.str();

    file.close();
    return (content);
}

/*
return an string for the content-type header corresponding to the requested file extension
*/
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

/*
returns an string with the current date and time
*/
static std::string getCurrentDateTime()
{
    time_t now = time(0);
    struct tm* timeinfo = gmtime(&now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
    return std::string(buffer);
}

/*
build and returns an default html error page with the error_code
*/
static std::string _buildDefaultErrorPage(int error_code)
{
    std::ostringstream  oss;

    oss << "<!DOCTYPE html><html><head><title>Error</title></head><center><h1>";
    oss << error_code << " " << lookupErrorMessage(error_code);
    oss << "</h1></center><hr><center>webserv</center></body></html>";
    return (oss.str());
}

/*
builds an html autoindex and returns it as a string
*/
static std::string buildAutoindex(std::string path_with_root, std::string path)
{
    std::vector<std::string> files;
    DIR* dir = opendir(path_with_root.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            files.push_back(entry->d_name);
        }
        closedir(dir);
    }
    std::ostringstream  oss;
    oss << "<!DOCTYPE html><html><head><title>Index of " << path << "</title></head><body><h1>Index of " << path << "</h1><hr><pre>";
    for (size_t i = 0; i < files.size(); ++i)
    {
        struct stat file_info;
        std::string full_path = path_with_root + "/" + files[i];
        if (stat(full_path.c_str(), &file_info) == 0)
        {
            oss << "<a href=\"";
            if (S_ISDIR(file_info.st_mode))
                oss << files[i] << '/';
            else
                oss << files[i];
            oss << "\">" << files[i] << "</a>" << "\t\t" << file_info.st_size << " bytes\n";
        }
    }
    oss << "</pre><hr></body></html>";
    return (oss.str());
}

// ======   Private member functions   ======= //
/*
sets _connection either to 'close' or 'keep-alive' dependinfg on _error and client request
*/
void Response::_setConnection(HttpRequest& request)
{
    _connection = "close";
    if (_error == OK)
    {
        std::map<std::string, std::string>::const_iterator it = request.getHeaders().find("Connection");
        if(it != request.getHeaders().end() && it->second == "keep-alive")
        {
            _connection = "keep-alive";
            return ;
        } 
    }
    return ;
}

/*
searches for custom error page or uses default error page to setuo _content string
*/
void Response::_setErrorPage(Server &server)
{
    std::map<int, std::string> error_pages = server.getErrorPages();

    if (error_pages.count(_error))
    {
        _content = readFile(error_pages[_error]);
        _content_type = getMimeType(error_pages[_error]);
    }
    else
    {
        _content = _buildDefaultErrorPage(_error);
        _content_type = "text/html";
    }
}

/*
handles an GET Request and sets all needed headers
*/
void Response::_handleGet(HttpRequest &request, Server &server)
{
    if (_error != OK)
        return ;

    std::string path = request.getPath();
    const std::map<std::string, location_t> locations = server.getLocations();
    std::map<std::string, location_t>::const_iterator location_it = locations.end();

    size_t size = 0;
    // search for location
    for (std::map<std::string, location_t>::const_iterator it = locations.begin(); it != locations.end(); it++)
    {
        if (path == it->first)
        {
            location_it = it;
            break ;
        }
        else if (path.compare(0, it->first.size(), it->first) == 0)
        {
            if (it->first.size() > size)
            {
                location_it = it;
                size = it->first.size();
            }
        }
    }
    // no location found
    if (location_it == locations.end())
    {
        _error = NOT_FOUND;
        return ;
    }
    // checks if GET is allowed
    if (location_it->second.allowed_methods.allow_get == false)
    {
        _error = NOT_ALLOWED;
        return ;
    }
    // check for redirection
    if (location_it->second.redirection != "")
    {
        _error = MOVED_PERMANENTLY;
        _location = location_it->second.redirection;
        return ;
    }
    // check if targt is there
    std::string full_path, root;
    root = server.getRoot();
    root.erase(root.size() - 1);
    if (location_it->second.alias != "")
    {
        size_t pos = path.find(location_it->first);

        if (pos != std::string::npos)
            path.replace(pos, location_it->first.size(), location_it->second.alias);
        full_path = path;
    }
    else
        full_path = root + path;
    struct stat file_info;
    // file does not exist
    if (stat(full_path.c_str(), &file_info) != 0)
    {
        _error = NOT_FOUND;
        return ;
    }
    // checks if targt is directory
    if (S_ISDIR(file_info.st_mode))
    {
        // Path does not ends with "/" or "/$"
        if (full_path[full_path.size() - 1] != '/' && full_path.compare(full_path.size() - 2, 2, "/$") != 0)
        {
            _error = NOT_FOUND;
            return;
        }
        // check for index
        if (location_it->second.index != "")
        {
            _content = readFile(location_it->second.index);
            _content_type = getMimeType(location_it->second.index);
            return ;
        }
        // check for autoindex
        if (location_it->second.autoindex == false)
        {
            _error = FORBIDDEN;
            return ;
        }
        else
        {
            _content = buildAutoindex(full_path, path);
            _content_type = "text/html";
            return ;
        }
    }
    // checks if target is regular file
    else if (S_ISREG(file_info.st_mode))
    {
        _content = readFile(full_path);
        _content_type = getMimeType(full_path);
        return ;
    }
    else
    {
        _error = NOT_FOUND;
        return ;
    }
}

/*
handles an POST Request and sets all needed headers
*/
void Response::_handlePost(HttpRequest &request, Server &server)
{
    if (_error != OK)
        return ;
    std::string path = request.getPath();
    const std::map<std::string, location_t> locations = server.getLocations();
    std::map<std::string, location_t>::const_iterator location_it = locations.end();

    size_t size = 0;
    // search for location
    for (std::map<std::string, location_t>::const_iterator it = locations.begin(); it != locations.end(); it++)
    {
        if (path == it->first)
        {
            location_it = it;
            break ;
        }
        else if (path.compare(0, it->first.size(), it->first) == 0)
        {
            if (it->first.size() > size)
            {
                location_it = it;
                size = it->first.size();
            }
        }
    }
    // no location found
    if (location_it == locations.end())
    {
        _error = NOT_FOUND;
        return ;
    }
    // checks if POST is allowed
    if (location_it->second.allowed_methods.allow_post == false)
    {
        _error = NOT_ALLOWED;
        return ;
    }
    // check for redirection
    if (location_it->second.redirection != "")
    {
        _error = MOVED_PERMANENTLY;
        _location = location_it->second.redirection;
        return ;
    }
    std::cout<< path << std::endl;
    std::string full_path, root;
    root = server.getRoot();
    root.erase(root.size() - 1);
    if (location_it->second.alias != "")
    {
        size_t pos = path.find(location_it->first);

        if (pos != std::string::npos)
            path.replace(pos, location_it->first.size(), location_it->second.alias);
        full_path = path;
    }
    else
        full_path = root + path;
    std::cout<< full_path << std::endl;
    struct stat file_info;
    // checks if targt is directory
    if (S_ISDIR(file_info.st_mode))
    {
        // check allowed methods
        // get request body
        // check if valid file
        // check if file already exists
        // save file to lacation
    }
    // checks if target is regular file
    else 
    {   
        if (S_ISREG(file_info.st_mode))
        {
            _content = readFile(full_path);
            _content_type = getMimeType(full_path);
            return ;
            // check allowed methods
            // getMimeType?
            // check if it can write to location
            // open file
            // write to file

        }
        else
            // check allowed methods
            // return(_buildDefaultErrorPage(400));
            // create file
            // open and write to it 
            // if not working return error
            return ; 
    }
    //get status codes and update content 
    std::cout << "POST REQUEST" << std::endl;
}

/*
handles an DELETE Request and sets all needed headers
*/
void Response::_handleDelete(HttpRequest &request, Server &server)
{
    (void)request;
    (void)server;
    std::cout << "DELETE REQUEST" << std::endl;
}

/*
builds the _response string with all needed headers and content
*/
void Response::_buildResponseStr(HttpRequest &request, Server &server)
{
    std::ostringstream oss;
    
    _setConnection(request);
    if (_error != OK && _error != MOVED_PERMANENTLY)
        _setErrorPage(server);
    oss << "HTTP/1.1 " << _error << " " << lookupErrorMessage(_error) << "\r\n";
    oss << "Server: Webserv\r\n";
    oss << "Date: " << getCurrentDateTime() << "\r\n";
    if (_content != "")
        oss << "Content-Length: " << _content.size() << "\r\n";
    if (_content_type != "")
        oss << "Content-Type: " << _content_type << "\r\n";
    if (_connection != "")
        oss << "Connection: " << _connection << "\r\n";
    if (_location != "")
        oss << "Location: " << _location << "\r\n";
    oss << "\r\n";
    if (_content != "")
        oss << _content << "\r\n";

    _response = oss.str();
}


// ======   Public member functions   ======= //
/*
clear the response object
*/
void Response::clear()
{
    _response = "";
    _error = OK;
    _connection = "";
    _content = "";
    _content_type = "";
    _date = "";
    _location = "";
}

/*
builds the Response for the request of the client
*/
void Response::buildResponse(HttpRequest &request, Server &server)
{
    _error = request.getError();
    switch (request.getMethod()) {

    case GET:
        _handleGet(request, server);
        break;
    case POST:
        _handlePost(request, server);
        break;
    case DELETE:
        _handleDelete(request, server);
        break;
    default:
        // ???
        break;
    }
    _buildResponseStr(request, server);
}
