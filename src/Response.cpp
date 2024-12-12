#include "../inc/Response.hpp"

// =============   Constructor   ============= //
Response::Response()
{
    _response = "";
    _error = OK;
    _connection = "";
    _content = "";
    _content_type = "";
    _location = "";
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

const std::string &Response::getConnection() const
{
    return _connection;
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
build and returns an default html error page with the error_code
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
    std::vector<std::string> files;
    DIR* dir = opendir(path_with_root.c_str());
    if (dir) 
    {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL)
            files.push_back(entry->d_name);
        closedir(dir);
    }
    std::ostringstream  oss;
    std::string path_without_root = path_with_root;
    path_without_root.erase(0, root.size());
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
static std::map<std::string, Location>::const_iterator   findLocation(std::string path, const std::map<std::string, Location> &locations)
{
    std::map<std::string, Location>::const_iterator location = locations.end();
    size_t size = 0;

    for (std::map<std::string, Location>::const_iterator it = locations.begin(); it != locations.end(); it++)
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

/*
gets success message
*/
static std::string    readStatus(int _error){
    switch (_error)
    {
    case OK:
        return("docs/saving_success.html");
    case CREATED:
        return("docs/new_user_success.html");
    case ACCEPTED:
        return("docs/upload_success.html");
    default:
        return "";
    }
}

// ======   Private member functions   ======= //
/*
sets _connection either to 'close' or 'keep-alive' dependinfg on _error and client request
*/
void Response::_setConnection(Request& request)
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
void Response::_setErrorPage(ServerBlock &server)
{
    std::map<int, std::string> error_pages = server._error_pages;

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
void Response::_handleGet(Request &request, ServerBlock &server)
{
    if (_error != OK)
        return ;

    std::map<std::string, Location>::const_iterator location;
    std::string path = request.getPath();

    location = findLocation(path, server._locations);
    
    // no location found
    if (location == server._locations.end())
    {
        _error = NOT_FOUND;
        return ;
    }
    // checks if GET is allowed
    if (location->second._allowed_methods._allow_get == false)
    {
        _error = NOT_ALLOWED;
        return ;
    }
    // check for redirection
    if (location->second._redirection != "")
    {
        _error = MOVED_PERMANENTLY;
        _location = location->second._redirection;
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
    // cgi
    // if(!location->second.cgi = "")
    // {
    //     process_cgi(char **cgifile, char **env, client);
    //     return ;
    //     if fd != -1 save in content
    //     else return error code
    // }
    // for (std::map<std::string, std::string>::const_iterator it = request.getHeaders().begin(); it != request.getHeaders().end(); ++it) {
    //     std::cout << it->first << ": " << it->second << std::endl;
    // }
    // std::cout << request.getPath() <<std::endl;
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
            _location = path + "/";
            return;
        }
        // check for index
        if (location->second._index != "")
        {
            _content = readFile(location->second._index);
            _content_type = getMimeType(location->second._index);
            return ;
        }
        // check for autoindex
        if (location->second._autoindex == false)
        {
            _error = FORBIDDEN;
            return ;
        }
        else
        {
            _content = buildAutoindex(path, root);
            _content_type = "text/html";
            return ;
        }
    }
    // checks if target is regular file
    else if (S_ISREG(file_info.st_mode))
    {
        _content = readFile(path);
        _content_type = getMimeType(path);
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
void Response::_handlePost(Request &request, ServerBlock &server)
{
    if (_error != OK)
        return ;

    std::map<std::string, Location>::const_iterator location;
    std::string path = request.getPath();

    location = findLocation(path, server._locations);

    // no location found
    if (location == server._locations.end())
    {
        _error = NOT_FOUND;
        return ;
    }
    // checks if GET is allowed
    if (location->second._allowed_methods._allow_post == false)
    {
        _error = NOT_ALLOWED;
        return ;
    }
    // check for redirection
    if (location->second._redirection != "")
    {
        _error = MOVED_PERMANENTLY;
        _location = location->second._redirection;
        return ;
    }

    std::string full_path, root;
    root = server._root;
    root.erase(root.size() - 1);

    full_path = root + path;
    struct stat file_info;

    if (stat(full_path.c_str(), &file_info) == 0 && S_ISDIR(file_info.st_mode)) // upload a file
    {
        size_t content_start;
        size_t content_end;
        std::string filename;
        std::string filepath;
    
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
            filepath = full_path + "/" + filename;
            content_end = request.getBody().find(boundary_end) - 2;
            content_start = request.getBody().find("\r\n\r\n") + 4;
        }
        else
        {
            if(server._locations.at("/uploads/")._allowed_methods._allow_post == false){
                _error = NOT_ALLOWED;
                return;
            }
            filename = getCurrentDateTime();
            full_path = server._locations.at("/uploads/")._alias;
            filepath = full_path + filename;
            content_start = 0;
            content_end = request.getBody().length();
        }
        std::ofstream outFile(filepath.c_str(), std::ios::binary);
        if(outFile.is_open()){
            outFile << request.getBody().substr(content_start, content_end - content_start);
            outFile.close();
            _error = ACCEPTED;
        }
        else{
            _error = INTERNAL_SERVER_ERROR;
            return ;}
        }
    
    else // checks if target is regular file
    {   
        // write to existing file
        if (stat(full_path.c_str(), &file_info) == 0 && S_ISREG(file_info.st_mode))
        {
            std::fstream outFile(full_path.c_str(), std::ios::binary | std::ios::in | std::ios::out | std::ios::app);
            if(outFile.is_open()){
                outFile << request.getBody();
                outFile.close();
            }
            else{
                _error = INTERNAL_SERVER_ERROR;
                return ;}
        }
        // create user file
        else{
            std::ofstream outFile(full_path.c_str(), std::ios::binary);
            if(outFile.is_open()){
                outFile << request.getBody();
                outFile.close();
                _error = CREATED;
            }
            else{
                _error = INTERNAL_SERVER_ERROR;
                return ;}
        }
    }
    std::string message_path = readStatus(_error);
    if(message_path.empty()){
        _error = INTERNAL_SERVER_ERROR;
        return ;}
    _content = readFile(message_path);
    _content_type = getMimeType(message_path);
    return ;
}

/*
handles an DELETE Request and sets all needed headers
*/
void Response::_handleDelete(Request &request, ServerBlock &server)
{
    if (_error != OK)
        return ;

    std::map<std::string, Location>::const_iterator location;
    std::string path = request.getPath();

    location = findLocation(path, server._locations);

    // no location found
    if (location == server._locations.end())
    {
        _error = NOT_FOUND;
        return ;
    }
    // checks if DELETE is allowed
    if (location->second._allowed_methods._allow_delete == false)
    {
        _error = NOT_ALLOWED;
        return ;
    }
    // check for redirection
    if (location->second._redirection != "")
    {
        _error = MOVED_PERMANENTLY;
        _location = location->second._redirection;
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
builds the _response string with all needed headers and content
*/
void Response::_buildResponseStr(Request &request, ServerBlock &server)
{
    std::ostringstream oss;
    
    _setConnection(request);
    if (_error != OK && _error != MOVED_PERMANENTLY && _error != CREATED && _error != ACCEPTED)
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
    _location = "";
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
void Response::buildResponse(Request &request)
{
    ServerBlock *server = request.getServerBlock();

    if (server == NULL)
        std::cout << "NULL du idiot" << std::endl;

    _error = request.getError();

    switch (request.getMethod()) {

    case GET:
        _handleGet(request, *server);
        break;
    case POST:
        _handlePost(request, *server);
        break;
    case DELETE:
        _handleDelete(request, *server);
        break;
    default:
        _error = NOT_IMPLEMENTED;
        break;
    }
    _buildResponseStr(request, *server);
}

/*
to do:
- build cgi connection - get content type
- content-length for cgi
- allow_post
- handle multiple file upload
- [11/Dec/2024  21:03:42]  [ERROR]  Could not bind socket: Address already in use -> when CRTL D
*/








// Info fuer Freddy
// FREE THE newenv after using it for your nefarious purposes
// close fdread when you are done with it
// check if fdread is -1, if so, system function has failed, Error Code 500

// cgifile muss so aufgebaut werden:
// <PATH>/python3
// <CGIFILE>
// <ARGUMENT 1 etc.>

// Takes two lines and allocates them into one, skips line 2 if its \0
char *newlinecombine(const char *line1, const char *line2) {
    char *newline = (char*)malloc(256 * sizeof(char));
    int i = 0; 
    int j = 0;
    while(line1[i]) {
        newline[i] = line1[i];
        i++;
    }
    while(line2[j]) {
        newline[i] = line2[j];
        i++;
        j++;
    }
    newline[i] = '\0';
    return newline;
}

char* itoa(int num) {
    char *str;
    int i = 10;
    int minus = 0;
    str = (char*)malloc(16 * sizeof(char));

    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return str;
    }
    if (num < 0) {
        minus = 1;
        num = -num;
    }
    str[11] = '\0';
    while (num != 0) {
        str[i] = (num % 10) + '0';
        num = num / 10;
        i--;
    }
    if (minus) {
        str[i] = '-';
        return &str[i];
    }
    return &str[i+1];
}

// /HttpRequest &ref1, Server &ref2, Response &ref3
char** Response::_buildenv(char *cgifile, char **env,int clientfd, Request &ref1, ServerBlock &ref2)
{ 
    char **newenv = (char **)malloc(400 * sizeof(char *));;

    int j = 0;
    for (int i = 0; env[i] ;i++) {
        newenv[i] =  newlinecombine(env[i], "\0");
        j++;
    }
    //ref 1 = request ||| ref 2 = Server
    newenv[j++] = newlinecombine("REDIRECT_STATUS=\0", itoa(_error));
    newenv[j++] = newlinecombine("CONTENT_TYPE=\0", _content_type.c_str()); //getContent_type existiert nicht
    newenv[j++] = newlinecombine("CONTENT_LENGTH=\0", itoa(_content.size())); // content length not defined in Response

    newenv[j++] = newlinecombine("GATEWAY_INTERFACE=\0", "CGI/1.1\0");
    newenv[j++] = newlinecombine("PATH_INFO=\0", ref1.getPath().c_str()); 
    newenv[j++] = newlinecombine("PATH_TRANSLATED=\0", (ref2._root + ref1.getPath()).c_str()); // path to CGI script but out of root
    newenv[j++] = newlinecombine("QUERY_STRING=\0", ref1.getQuery().c_str());
    // newenv[j++] = newlinecombine("REMOTE_ADDR=\0", sockaddrToIpString(ref1.getClientAddress()).c_str()); //client shit
    newenv[j++] = newlinecombine("REMOTE_HOST=\0", ""); // not defined in Client, empty because unlikely
    newenv[j++] = newlinecombine("REMOTE_USER=\0", itoa(clientfd));
    newenv[j++] = newlinecombine("REQUEST_METHOD=\0", itoa(ref1.getMethod()));
    newenv[j++] = newlinecombine("SCRIPT_NAME=\0", cgifile);
    if (ref2._server_names.size())
        newenv[j++] = newlinecombine("SERVER_NAME=\0", ref2._server_names[0].c_str());
    else
        newenv[j++] = newlinecombine("SERVER_NAME=\0", "");
    newenv[j++] = newlinecombine("SERVER_PORT=\0", itoa(ref2._port));
    newenv[j++] = newlinecombine("SERVER_PROTOCOL=\0", "HTTP/1.1\0");
    newenv[j++] = newlinecombine("SERVER_SOFTWARE=\0", "Webserv/1.0\0");
    newenv[j] = 0;
    return newenv;
}

//we return an int, which is a file descriptor where everything has been dumped into.
CgiReturn* Response::_process_cgi(char **cgifile, char **env, int clientfd, Request &ref1, ServerBlock &ref2) 
{
    CgiReturn *data;
    data = (CgiReturn*)malloc(2 * sizeof(CgiReturn));
    _buildenv(cgifile[1], env, clientfd, ref1, ref2);
    int fd[2], pid, status;
    if (pipe(fd) == -1) {
        Logger::log(RED, ERROR, "Creating pipe has failed, aborting CGI init process.");
        data->fdread = -1;
        return data;
    }
    if ((pid = fork()) == -1) {
        Logger::log(RED, ERROR, "Creating fork has failed, aborting CGI init process.");
        data->fdread = -1;
        return data;
    }
    if (!pid) {
        if (dup2(fd[1], 1) == -1) {
            Logger::log(RED, ERROR, "Child Process ID: %i: dup2 has failed (IN)", pid);
            abort();
        }
        if (dup2(fd[0], 0) == -1) {
            Logger::log(RED, ERROR, "Child Process ID: %i: dup2 has failed (OUT)", pid);
            abort();
        }
        execve(*cgifile, cgifile, env);
        abort();
        Logger::log(RED, ERROR, "Child Process ID: %i: Execve has failed. Program name: %s", pid, *cgifile);
    }
    waitpid(pid, &status, 0);
    if (!WIFEXITED(status)) {
       data->fdread = -1;
       return data;
    }
    close(fd[1]); //WRITE END
    //close(fd[0]); //READ END
    data->fdread = fd[0];
    return data;
}
