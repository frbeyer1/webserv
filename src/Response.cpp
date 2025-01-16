#include "../inc/Response.hpp"

// =============   Constructor   ============= //
Response::Response()
{
    _response = "";
    _error = OK;
    _body = "";
}

// ============   Deconstructor   ============ //
Response::~Response()
{
}

// ==============   Getters   ================ //
const std::string &Response::getResponse() const
{
    return _response;
}

int Response::getError() const
{
    return _error;
}

// ================   Utils   ================ //
/*
returns an string accordingly to the error_code
*/
static  std::string lookupErrorMessage(int error_code)
{
    switch (error_code) {
    case  200: return "OK";
    case  201: return "Created";
    case  202: return "Accepted";
    case  301: return "Moved Permanently";
    case  302: return "Found";
    case  303: return "See Other";
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
    return content;
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
        mimeTypes[".json"] = "application/json";
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
transforms an int into an string
*/
std::string intToStr(int n)
{
    std::stringstream ss;
    ss << n;
    return ss.str();
}

/*
build and returns an default html page with the error_code
*/
static std::string _buildDefaultErrorPage(int error_code)
{   
    std::ostringstream  oss;

    oss << "<!DOCTYPE html><html><head><title>Error</title></head><center><h1>";
    oss << error_code << " " << lookupErrorMessage(error_code);
    oss << "</h1></center><hr><center>webserv</center></body></html>";
    return oss.str();
}

/*
builds an html autoindex and returns it as a string
*/
static std::string buildAutoindex(std::string path_with_root, std::string root)
{
    std::ostringstream  oss;
    std::string path_without_root = path_with_root;
    path_without_root = "/" + path_without_root.erase(0, root.size());
    std::vector<std::string> files;
    DIR* dir = opendir(path_with_root.c_str());
    if (dir) 
    {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL)
            files.push_back(entry->d_name);
        closedir(dir);
    }
    else
        Logger::log(RED, ERROR, "Could not open directory: %s", path_with_root);
    oss << "<!DOCTYPE html><html><head><title>Index of " << path_without_root << "</title></head><body><h1>Index of " << path_without_root << "</h1><hr><pre>";
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
    return oss.str();
}

/*
searches the right location for the request
*/
static std::map<std::string, Location>::iterator   findLocation(std::string path, std::map<std::string, Location> &locations)
{
    std::map<std::string, Location>::iterator location = locations.end();
    size_t size = 0;

    for (std::map<std::string, Location>::iterator it = locations.begin(); it != locations.end(); it++)
    {
        if (path == it->first)
        {
            location = it;
            break ;
        }
        else if (path.compare(0, it->first.size(), it->first) == 0)
        {
            if (it->first.size() > size)
            {
                location = it;
                size = it->first.size();
            }
        }
    }
    return location;
}

// ======   Private member functions   ======= //
/*
sets _connection either to 'close' or 'keep-alive' depending on _error and client request
*/
void Response::_setConnection(Request& request)
{
    if (_error < 400)
    {
        std::map<std::string, std::string>::const_iterator it = request.getHeaders().find("Connection");
        if(it != request.getHeaders().end() && it->second == "keep-alive")
        {
            _headers.insert(std::make_pair("Connection", "keep-alive"));
            return;
        } 
    }
    _headers.insert(std::make_pair("Connection", "close"));
    return;
}

/*
searches for custom error page or uses default error page to setup the _body string
*/
void Response::_buildErrorPage(ServerBlock &server)
{
    std::map<int, std::string> error_pages = server._error_pages;

    _body.clear();
    if (error_pages.count(_error))
    {
        _body = readFile(error_pages[_error]);
        _headers.insert(std::make_pair("Content-Type", getMimeType(error_pages[_error])));
    }
    else
    {
        _body = _buildDefaultErrorPage(_error);
        _headers.insert(std::make_pair("Content-Type", "text/html"));
    }
}

/*
handles an GET request
*/
void Response::_handleGet(ServerBlock &server, std::string path, Location &location)
{
    struct stat file_info;
    
    // file does not exist
    if (stat(path.c_str(), &file_info) != 0)
    {
        _error = NOT_FOUND;
        return ;
    }
    // checks if targt is directory
    if (S_ISDIR(file_info.st_mode))
    {
        // Path does not ends with "/" or "/$"
        if (path[path.size() - 1] != '/' && path.compare(path.size() - 2, 2, "/$") != 0)
        {
            _error = MOVED_PERMANENTLY;
            _headers.insert(std::make_pair("Location", path + "/"));
            return;
        }
        // check for index
        if (location._index != "")
        {
            _body = readFile(location._index);
            _headers.insert(std::make_pair("Content-Type", getMimeType(location._index)));
            return ;
        }
        // check for autoindex
        if (location._autoindex == false)
        {
            _error = FORBIDDEN;
            return ;
        }
        else
        {
            _body = buildAutoindex(path, server._root);
            _headers.insert(std::make_pair("Content-Type", "text/html"));
            return ;
        }
    }
    // checks if target is regular file
    else if (S_ISREG(file_info.st_mode))
    {
        _body = readFile(path);
        _headers.insert(std::make_pair("Content-Type", getMimeType(path)));
        return ;
    }
    else
    {
        _error = NOT_FOUND;
        return ;
    }
}

/*
handles an POST request
*/
void Response::_handlePost(Request &request, std::string path, Location &location)
{
    if (location._upload.empty())
    {
        _error = FORBIDDEN;
        return;
    }
    struct stat file_info;

    if (stat(path.c_str(), &file_info) == 0 && S_ISDIR(file_info.st_mode)) // upload a file
    {
        size_t content_start;
        size_t content_end;
        std::string filename;
        std::string filepath;
        filepath = location._upload;
    
        if(request.getHeaders().at("Content-Type").find("multipart/form-data") != std::string::npos)
        {
            std::istringstream file_content(request.getBody());
            std::string boundary;
            std::getline(file_content, boundary);
            boundary.erase(boundary.find_last_not_of("\r\n") + 1);
            std::string boundary_end = boundary;
            boundary_end.append("--");
            size_t pos = request.getBody().find("filename") + 10;
            if (pos != std::string::npos)
            {
                size_t filename_end = request.getBody().find('\"', pos);
                if (filename_end != std::string::npos){
                    filename = request.getBody().substr(pos, filename_end - pos);
                }
                else{
                    _error = BAD_REQUEST;
                    return ;}
            }
            else{
                _error = BAD_REQUEST;
                return ;}
            
            filepath = filepath + "/" + filename;
            content_end = request.getBody().find(boundary_end) - 2;
            content_start = request.getBody().find("\r\n\r\n") + 4;
        }
        else
        {
            filename = getCurrentDateTime();
            path = location._alias;
            filepath = filepath + filename;
            content_start = 0;
            content_end = request.getBody().length();
        }
        std::ofstream outFile(filepath.c_str(), std::ios::binary);
        if(outFile.is_open()){
            outFile << request.getBody().substr(content_start, content_end - content_start);
            outFile.close();
            _error = CREATED;
            return;
        }
        else
        {
            _error = INTERNAL_SERVER_ERROR;
            return;
        }
    }
    else // checks if target is regular file
    {   
        // write to existing file
        if (stat(path.c_str(), &file_info) == 0 && S_ISREG(file_info.st_mode))
        {
            std::fstream outFile(path.c_str(), std::ios::binary | std::ios::in | std::ios::out | std::ios::app);
            if(outFile.is_open()){
                outFile << request.getBody();
                outFile.close();
            }
            else{
                _error = INTERNAL_SERVER_ERROR;
                return ;}
        }
        // create new file
        else{
            std::ofstream outFile(path.c_str(), std::ios::binary);
            if(outFile.is_open()){
                outFile << request.getBody();
                outFile.close();
                _error = CREATED;
            }
            else
            {
                _error = INTERNAL_SERVER_ERROR;
                return ;
            }
        }
    }
    return ;
}

/*
handles an DELETE request
*/
void Response::_handleDelete(std::string path)
{
    struct stat file_info;
    
    // file does not exist
    if (stat(path.c_str(), &file_info) != 0)
    {
        _error = NOT_FOUND;
        return ;
    }
    // check for write accsess
    if ((file_info.st_mode & S_IWUSR) == 0)
    {
        _error = FORBIDDEN;
        return ;
    }
    // checks if targt is directory
    if (S_ISDIR(file_info.st_mode))
    {
        // Path does not ends with "/" or "/$"
        if (path[path.size() - 1] != '/' && path.compare(path.size() - 2, 2, "/$") != 0)
        {
            _error = NOT_FOUND;
            return;
        }
    }
    // removes file
    if (remove(path.c_str()) != 0)
    {
        _error = INTERNAL_SERVER_ERROR;
        return ;
    }
}

/*
checks if the request needs cgi:
    - returns true and executes cgi if cgi is necessary
    - returns false if no cgi is needed
*/
bool Response::_checkCgi(Request &request, ServerBlock &server, std::string path, Location &location)
{
    if (_error != OK)
        return false;

    // checks if method is working with cgi
    if (request.getMethod() != GET && request.getMethod() != POST)
        return false;

    // checks if cgi is allowed
    if (location._cgi.size() == 0)
        return false;
    
    struct stat file_info;
    
    // file does not exist
    if (stat(path.c_str(), &file_info) != 0)
        return false;

    // checks if targt is directory
    if (S_ISDIR(file_info.st_mode))
    {
        // Path does not ends with "/" or "/$"
        if (path[path.size() - 1] != '/' && path.compare(path.size() - 2, 2, "/$") != 0)
            return false;
        // check for index
        if (location._index != "")
            path = location._index;
        else
            return false;
    }
    // checks if target is regular file
    else if (!S_ISREG(file_info.st_mode))
        return false;
    // check cgi_file extension is allowed
    size_t pos = path.find_last_of('.');
    if (pos == std::string::npos)
        return false;
    
    std::string extension = path.substr(pos, path.size() - pos);
    if (location._cgi.count(extension))
    {
        // handle cgi
        CgiHandler cgi(request, server, path, location._cgi[extension], _client_addr);

        cgi.execCgi();
        _body = cgi.getBody();
        _headers.insert(cgi.getHeaders().begin(), cgi.getHeaders().end());
        if (cgi.getError() != OK)
            _error = cgi.getError();
        return true;
    }
    return false;
}

/*
checks the request for location, allowed_method, redirection, and alias
    calls _checkCgi() -> if no cgi: calls the function to handle the request with right method
*/
void Response::_handleRequest(Request &request, ServerBlock &server)
{
    std::map<std::string, Location>::iterator location;
    std::string path = request.getPath();

    location = findLocation(path, server._locations);
    
    // no location found
    if (location == server._locations.end())
    {
        _error = NOT_FOUND;
        return ;
    }
    // checks if Method is allowed
    switch (request.getMethod()) {

    case GET:
        if (location->second._allowed_methods._allow_get == false)
        {
            _error = NOT_ALLOWED;
            return;
        }
        break;
    case POST:
        if (location->second._allowed_methods._allow_post == false)
        {
            _error = NOT_ALLOWED;
            return;
        }
        break;
    case DELETE:
        if (location->second._allowed_methods._allow_delete == false)
        {
            _error = NOT_ALLOWED;
            return;
        }
        break;
    default:
        _error = NOT_IMPLEMENTED;
        return;
    }

    // check for redirection
    if (location->second._redirection != "")
    {
        _error = MOVED_PERMANENTLY;
        _headers.insert(std::make_pair("Location", location->second._redirection));
        return ;
    }

    // add root to path
    std::string root;
    root = server._root;
    root.erase(root.size() - 1, 1);
    path = root + path;

    // check for alias
    if (location->second._alias != "")
    {
        size_t pos = path.find(root + location->first);
        if (pos != std::string::npos)
            path.replace(pos, root.size() + location->first.size(), location->second._alias);
    }

    // check for cgi
    if (_checkCgi(request, server, path, location->second))
        return ;

    // handle methods
    switch (request.getMethod()) {

    case GET:
        _handleGet(server, path, location->second);
        break;
    case POST:
        _handlePost(request, path, location->second);
        break;
    case DELETE:
        _handleDelete(path);
        break;
    default:
        _error = NOT_IMPLEMENTED;
        return;
    }
}

// ======   Public member functions   ======= //
/*
looks inside the response headers for "Connection"
    - returns true if Connection: keep-alive
    - returns false in all other cases
*/
bool Response::checkConnection()
{
    if (_headers.count("Connection"))
    {
        if (_headers["Connection"] == "keep-alive")
            return true;
    }
    return false;
}

/*
clear the response object
*/
void Response::clear()
{
    _response = "";
    _error = OK;
    _body = "";
    _headers.clear();
}

/*
trims the response string by the allready send bytes
*/
void Response::trimResponse(int bytes_send)
{
    _response.erase(0, bytes_send);
}

/*
builds the Response for the request of the client
*/
void Response::buildResponse(Request &request, sockaddr_in client_addr)
{
    // getting server block
    ServerBlock *server = request.getServerBlock();

    if (server == NULL)
        return ;

    _client_addr = client_addr;
    _error = request.getError();
    if (_error == OK)
        _handleRequest(request, *server);

    if (_error >= 400 || _error == CREATED)
        _buildErrorPage(*server);
    
    // setting headers
    if (!_body.empty())
        _headers.insert(std::make_pair("Content-Length", intToStr(_body.size())));
    _setConnection(request);
    _headers.insert(std::make_pair("Server", "Webserv"));
    _headers.insert(std::make_pair("Date", getCurrentDateTime()));
    
    // building response string
    std::ostringstream oss;

    // insert header line 
    oss << "HTTP/1.1 " << _error << " " << lookupErrorMessage(_error) << "\r\n";

    // insert headers
    for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); it++)
        oss << it->first << ": " << it->second << "\r\n";
    oss << "\r\n";

    // insert body
    if (!_body.empty())
        oss << _body << "\r\n";

    _response = oss.str();
}

