#include "../inc/ConfigParser.hpp"

// =============   Constructor   ============= //
ConfigParser::ConfigParser(std::vector<ServerBlock> &server_blocks) : _server_blocks(server_blocks)
{
    _content = "";
    _i = 0;
}

// ============   Deconstructor   ============ //
ConfigParser::~ConfigParser()
{
}

// ================   Utils   ================ //
/*
converts an string ip into an numeric ip address
!!! throws exception at fail !!!
*/
uint32_t ipStringToNumeric(const std::string& ip)
{
    std::stringstream   ss(ip);
    std::string         segment;
    uint32_t            numericIp = 0;
    int                 segmentCount = 0;

    while (std::getline(ss, segment, '.'))
    {
        if (segmentCount >= 4)
            throw std::invalid_argument("IP address has too many octets.");
        int segmentValue = std::atoi(segment.c_str());
        if (segmentValue < 0 || segmentValue > 255)
            throw std::invalid_argument("Octet is out of range");
        numericIp = (numericIp << 8) | segmentValue;
        segmentCount++;
    }
    if (segmentCount != 4)
        throw std::invalid_argument("IP address has too less octets.");
    return numericIp;
}

/*
returns an IP string from an in_addr_t
*/
std::string inAddrToIpString(in_addr_t addr)
{
    unsigned char* ip = (unsigned char*)&addr;
    std::ostringstream oss;

    oss << (int)ip[0] << "." 
        << (int)ip[1] << "." 
        << (int)ip[2] << "." 
        << (int)ip[3];
    
    return oss.str();
}

/*
parses an parameter string of the config and sets root on the corresponding server_block
*/
static void handleRoot(std::string parameter, ServerBlock &server_block)
{
    struct stat buf;

    if (parameter[parameter.size() - 1] != '/')
    {
        Logger::log(RED, ERROR, "Error: config file misconfigured: root directive: missing '/' at end");
        exit(EXIT_FAILURE);
    }
    if (stat(parameter.c_str(), &buf) != 0)
    {
        Logger::log(RED, ERROR, "Error: Config file misconfigured: root directive: path invalid");
        exit(EXIT_FAILURE);
    }
    if (S_ISDIR(buf.st_mode))
        server_block._root = parameter;
    else
    {
        Logger::log(RED, ERROR, "Config file misconfigured: root directive: is no directory");
        exit(EXIT_FAILURE);
    }
}

/*
parses an parameter string of the config and sets port and host on the corresponding server_block
*/
static void handleListen(std::string parameter, ServerBlock &server_block)
{
    std::string port_str;
    std::string ip_str;
    bool        found_host = false;
    in_addr_t   host;
    int         port;

    for (size_t i = 0; i < parameter.length(); i++)
    {
        if (parameter[i] == ':')
        {
            found_host = true;
            if (parameter.compare(0, i, "localhost") == 0)
                ip_str = "127.0.0.1";
            else
                ip_str = parameter.substr(0, i);
            port_str = parameter.substr(i + 1, parameter.length());
            break ;
        }
    }
    if (!found_host)
    {
        port_str = parameter;
        ip_str = DEFAULT_HOST;
    }
    for (size_t i = 0; i < ip_str.length(); i++)
    {
        if (!isdigit(ip_str[i]) && ip_str[i] != '.')
        {
            Logger::log(RED, ERROR, "Config file misconfigured: listen directive: IP invalid");
            exit(EXIT_FAILURE);
        }
    }
    try
    {
        host = ipStringToNumeric(ip_str);
    }
    catch(const std::exception& e)
    {
        Logger::log(RED, ERROR, "Config file misconfigured: listen directive: IP invalid: %s", e.what());
        exit(EXIT_FAILURE);
    }
    server_block._host = host;
    server_block._ip = ip_str;
    for (size_t i = 0; i < port_str.length(); i++)
    {
        if (!isdigit(port_str[i]))
        {
            Logger::log(RED, ERROR, "Config file misconfigured: listen directive: port invalid");
            exit(EXIT_FAILURE);
        }
    }
    port = atoi(port_str.c_str());
    if (port < 1 || port > 65636)
    {
        Logger::log(RED, ERROR, "Config file misconfigured: listen directive: port invalid");
        exit(EXIT_FAILURE);
    }
    server_block._port = port;
}

/*
parses an parameter string of the config and sets the server_name on the corresponding server_block
valid characters:
    - Letters (a-z, A-Z)
    - Digits (0-9)
    - Hyphens (-)
    - Periods (.)
    - Tildes (~)
    - Underscores (_)
*/
static void handleServerNames(std::string parameter, ServerBlock &server_block)
{
    std::string name;

    for (size_t i = 0; i < parameter.length(); i++)
    {
        if (iswspace(parameter[i]))
        {
            server_block._server_names.push_back(name);
            name.clear();
            while (iswspace(parameter[i]))
                i++;
        }
        else if ((parameter[i] < 'a' || parameter[i] > 'z') && (parameter[i] < 'A' || parameter[i] > 'Z') 
            && (parameter[i] < '0' || parameter[i] > '9') && parameter[i] != '.' && parameter[i] != '-' && parameter[i] != '~' && parameter[i] != '_' )
        {
            Logger::log(RED, ERROR, "Config file misconfigured: server_name directive: invalid character");
            exit(EXIT_FAILURE);
        }
        else
            name.push_back(parameter[i]);
    }
    if (!name.empty())
        server_block._server_names.push_back(name);
}

/*
parses an parameter string of the config and sets the client_max_body_size on the corresponding server_block
*/
static void handleClientMaxBodySize(std::string parameter, ServerBlock &server_block)
{
    size_t  size;

    for (size_t i = 0; i < parameter.length(); i ++)
    {
        if (!isdigit(parameter[i]))
        {
            Logger::log(RED, ERROR, "Config file misconfigured: client_max_body_size directive: invalid character");
            exit(EXIT_FAILURE);
        }
    }
    size = atoi(parameter.c_str());
    server_block._client_max_body_size = size;
}

/*
parses an parameter string of the config and sets custom error pages on the corresponding server_block
*/
static void handleErrorPage(std::string parameter, ServerBlock &server_block)
{
    int         status_code;
    std::string status_code_str;
    std::string page_path;
    struct stat buf;
    size_t i = 0;

    for (; i < parameter.length(); i++)
    {
        if (i == 3)
            break ;
        if (isdigit(parameter[i]))
            status_code_str.push_back(parameter[i]);
        else
        {
            Logger::log(RED, ERROR, "Config file misconfigured: error_page directive: status code invalid");
            exit(EXIT_FAILURE);
        }
    }
    status_code = atoi(status_code_str.c_str());
    if (i != 3 || status_code < 100 || status_code > 599)
    {
        Logger::log(RED, ERROR, "Config file misconfigured: error_page directive: status code invalid");
        exit(EXIT_FAILURE);
    }
    for (; i < parameter.length(); i++)
    {
        if (i == 3)
        {
            if (!isspace(parameter[i]))
            {
                Logger::log(RED, ERROR, "Config file misconfigured: error_page directive: missing space");
                exit(EXIT_FAILURE);
            }
        }
        else
            page_path.push_back(parameter[i]);
    }
    page_path = server_block._root + page_path;
    if (stat(page_path.c_str(), &buf) != 0 || S_ISREG(buf.st_mode) == 0)
    {
        Logger::log(RED, ERROR, "Config file misconfigured: error_page directive: error page path invalid");
        exit(EXIT_FAILURE);
    }
    if (access(page_path.c_str(), R_OK))
    {
        Logger::log(RED, ERROR, "Config file misconfigured: error_page directive: error page has no read rights");
        exit(EXIT_FAILURE);
    }
    if (server_block._error_pages.count(status_code))
        server_block._error_pages.erase(status_code);
    server_block._error_pages.insert(std::pair<int, std::string>(status_code, page_path));
}

/*
parses an parameter string and look for vaild Http Methods and adds it to location
*/
void handleAllowedMethods(std::string parameter, Location &location)
{
    std::string  method;
    int         start = 0;

    for (size_t i = 0; i <= parameter.length(); i++)
    {
        if (iswspace(parameter[i]) || i == parameter.length())
        {
            method = parameter.substr(start, i - start);
            if (method == "GET")
                location._allowed_methods._allow_get = true;
            else if (method == "POST")
                location._allowed_methods._allow_post = true;
            else if (method == "DELETE")
                location._allowed_methods._allow_delete = true;
            else
            {
                Logger::log(RED, ERROR, "Config file misconfigured: allowed_method directive: invalid method");
                exit(EXIT_FAILURE);
            }
            while (parameter.length() && iswspace(parameter[i]))
                i++;
            start = i;
        }
    }
}

/*
sets the redirection string of the location
*/
static void handleRedirection(std::string parameter, Location &location)
{
    location._redirection = parameter;
}

/*
Checks the alias parameter:
    - is the parameter an directory
    - does the directory have read rights
*/
static void handleAlias(std::string parameter, Location &location, ServerBlock &server_block)
{
    std::string alias_path = server_block._root + parameter;
    struct stat buf;

    if (alias_path[alias_path.size() - 1] != '/')
    {
        Logger::log(RED, ERROR, "Error: config file misconfigured: alias directive: missing '/' at end");
        exit(EXIT_FAILURE);
    }
    if (stat(alias_path.c_str(), &buf) != 0)
    {
        Logger::log(RED, ERROR, "Error: Config file misconfigured: alias directive: path invalid");
        exit(EXIT_FAILURE);
    }
    if (S_ISDIR(buf.st_mode))
    {
        if (access(alias_path.c_str(), R_OK))
        {
            Logger::log(RED, ERROR, "Config file misconfigured: alias directive: directory has no read rights");
            exit(EXIT_FAILURE);
        }
        location._alias = alias_path;
    }
    else
    {
        Logger::log(RED, ERROR, "Config file misconfigured: alias directive: is no directory");
        exit(EXIT_FAILURE);
    }
}

/*
Checks the autoindex parameter:
 - either "on" orr "off"
*/
static void handleAutoIndex(std::string parameter, Location &location)
{
    if (parameter == "off")
        location._autoindex = false;
    else if (parameter == "on")
        location._autoindex = true;
    else
    {
        Logger::log(RED, ERROR, "Config file misconfigured: autoindex directive: invalid parameter (either 'on' or 'off')");
        exit(EXIT_FAILURE);
    }
}

/*
Checks the index parameter inside an location:
 - is the file there? (dependent on root)
 - and does it has read rights?
*/
static void handleIndex(std::string parameter, Location &location, ServerBlock &server_block)
{
    std::string index = server_block._root + parameter;
    struct stat buf;

    if (stat(index.c_str(), &buf) != 0 || S_ISREG(buf.st_mode) == 0)
    {
        Logger::log(RED, ERROR, "Config file misconfigured: index directive: index file is invalid");
        exit(EXIT_FAILURE);
    }
    if (access(index.c_str(), R_OK))
    {
        Logger::log(RED, ERROR, "Config file misconfigured: index directive: index file has no read rights");
        exit(EXIT_FAILURE);
    }
    location._index = index;
}

/*
Checks the upload directive:
 - checks if file is there and an directory
 - checks for write rights for the directory 
*/
static void handleUpload(std::string parameter, Location &location, ServerBlock &server_block)
{
    std::string upload_path = server_block._root + parameter;
    struct stat buf;

    if (stat(upload_path.c_str(), &buf) != 0)
    {
        Logger::log(RED, ERROR, "Error: Config file misconfigured: upload directive: path invalid");
        exit(EXIT_FAILURE);
    }
    if (S_ISDIR(buf.st_mode))
    { 
        if (access(upload_path.c_str(), W_OK))
        {
            Logger::log(RED, ERROR, "Config file misconfigured: upload directive: directory has no write rights");
            exit(EXIT_FAILURE);
        }
        location._upload = upload_path;
    }  
    else
    {
        Logger::log(RED, ERROR, "Config file misconfigured: upload directive: is no directory");
        exit(EXIT_FAILURE);
    }
}

/*
sets cgi path and extension to the location
*/
static void handleCgi(std::string parameter, Location &location)
{
    std::string extension, path;
    size_t i = 0;

    if (parameter[0] != '.')
    {
        Logger::log(RED, ERROR, "Config file misconfigured: cgi directive: expected '.' for the extension");
        exit(EXIT_FAILURE);
    }
    while (i < parameter.size())
    {
        if (isspace(parameter[i]))
            break ;
        extension.push_back(parameter[i]);
        i++;
    }
    if (!iswspace(parameter[i]))
    {
        Logger::log(RED, ERROR, "Config file misconfigured: cgi directive: missing space after extension");
        exit(EXIT_FAILURE);
    }
    while (iswspace(parameter[i]))
        i++;
    while (i < parameter.size())
    {
        path.push_back(parameter[i]);
        i++;
    }
    if (access(path.c_str(), F_OK))
    {
        Logger::log(RED, ERROR, "Config file misconfigured: cgi directive: can not find cgi at %s", path.c_str());
        exit(EXIT_FAILURE);
    }
    if (access(path.c_str(), X_OK))
    {
        Logger::log(RED, ERROR, "Config file misconfigured: cgi directive: can not execute cgi at %s", path.c_str());
        exit(EXIT_FAILURE);
    }
    location._cgi.insert(std::make_pair(extension, path));
}

// ======   Private member functions   ======= //
/*
Tries to open the config file, reads it and saves its content inside the _content string.
- the config string must be an path to the file
- config file must end with .conf
*/
void    ConfigParser::_readConfig(std::string config)
{
    std::ifstream           file(config.c_str());
    std::stringstream       buffer;
    std::string             extension = ".conf";

    // checks file extension
    if (config.size() < extension.size())
    {
        Logger::log(RED, ERROR, "Config must have '.conf' file extension: %s", config.c_str());
        exit(EXIT_FAILURE);
    }
    if (config.compare(config.size() - extension.size(), extension.size(), extension))
    {
        Logger::log(RED, ERROR, "Config must have '.conf' file extension: %s", config.c_str());
        exit(EXIT_FAILURE);
    }

    // reads file
    if (file.fail())
    {
        Logger::log(RED, ERROR, "Unable to open file: %s", config.c_str());
        exit(EXIT_FAILURE);
    }
    buffer << file.rdbuf();
    file.close();
    _content = buffer.str();
    Logger::log(GREY, DEBUG, "Finished with reading file: %s", config.c_str());
}

/*
skips comments in the _content string
*/
void    ConfigParser::_skipComment()
{
    if (_content[_i] == '#')
    {
        while (_i < _content.length() && _content[_i] != '\n')
            _i++;
        if (_content[_i] == '\n')
            _i++;
        _skipWhiteSpaces();
        return ;
    }
}

/*
skips whitespaces (and comments) in the _content string
*/
void    ConfigParser::_skipWhiteSpaces()
{
    for (; _i < _content.length(); _i++)
    {
        _skipComment();
        if (!iswspace(_content[_i]))
            return ;
    }
}

/*
goes through the _content string and serarches for the next server Block
*/
void    ConfigParser::_findNextServerBlock()
{
    for (; _i < _content.length(); _i++)
    {
        _skipWhiteSpaces();
        if (_content.compare(_i, 6, "server") == 0 || _content.compare(_i, 6, "Server") == 0)
        {
            _i += 6;
            _skipWhiteSpaces();
            if (_content[_i] == '{')
            {
                _i++;
                return ;
            }
            else
            {
                Logger::log(RED, ERROR, "Config file misconfigured: missing '{'");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            // std::cout << _content[_i];
            Logger::log(RED, ERROR, "Config file misconfigured: found something else than server block");
            exit(EXIT_FAILURE);
        }
    }
}

/*
goes forward in the the _content string and returns the parameter of an directive as a string
- returns as much chars until ';' is found 
*/
std::string ConfigParser::_getParameter()
{
    std::string parameter;
    size_t      start;
    bool        found;

    found = false;
    start = _i;
    for (; _i < _content.length(); _i++)
    {
        if (_content[_i] == ';')
        {
            if (iswspace(_content[_i - 1]))
            {
                Logger::log(RED, ERROR, "Config file misconfigured: invalid syntax: found whitespace before ';'");
                exit(EXIT_FAILURE);
            }
            _i++;
            found = true;
            break;
        }
        if (_content[_i] == '\n')
            break;
    }
    if (!found)
    {
        Logger::log(RED, ERROR, "Config file misconfigured: missing ';'");
        exit(EXIT_FAILURE);
    }
    parameter = _content.substr(start, _i - start - 1);
    return parameter;
}

/*
goes forward in the _content string and returns the directive type
*/
Directive ConfigParser::_getDirectiveType()
{
    std::map<std::string, Directive> map;
    map["root"] = ROOT;
    map["listen"] = LISTEN;
    map["server_name"] = SERVER_NAME;
    map["client_max_body_size"] = CLIENT_MAX_BODY_SIZE;
    map["error_page"] = ERROR_PAGE;
    map["allowed_methods"] = ALLOWED_METHODS;
    map["return"] = REDIRECTION;
    map["alias"] = ALIAS;
    map["autoindex"] = AUTOINDEX;
    map["index"] = INDEX;
    map["location"] = LOCATION;
    map["upload"] = UPLOAD;
    map["cgi"] = CGI;
    for (std::map<std::string, Directive>::const_iterator it = map.begin(); it != map.end(); it++)
    {
        const std::string&  keyword = it->first;
        Directive           type = it->second;

        if (_content.compare(_i, keyword.length(), keyword) == 0)
        {
            _i += keyword.length();
            if (_content[_i] == ' ')
                return type;
            else
                break ;
        }
    }
    return UNKNOWN;
}

/*
returns the path for the location
*/
std::string    ConfigParser::_getLocationPath()
{
    std::string path;

    for (; _i < _content.length(); _i++)
    {
        if (iswspace(_content[_i]))
            break;
        else
            path.push_back(_content[_i]);
    }
    return path;
}

/*
parses the location directive and adds it to the corresponding server_block
*/
void    ConfigParser::_getLocation(ServerBlock &server_block)
{
    Directive   type;
    Location    location;
    std::string parameter;
    std::string path;
    bool        not_empty = false;

    std::memset(&location._allowed_methods, 0, sizeof(AllowedMethods));
    std::memset(&location._autoindex, 0, sizeof(bool));
    path = _getLocationPath();
    _skipWhiteSpaces();
    if (_content[_i] != '{')
    {
        Logger::log(RED, ERROR, "Config file misconfigured: missing '{'");
        exit(EXIT_FAILURE);
    }
    _i++;
    for (; _i < _content.length(); _i++)
    {
        _skipWhiteSpaces();
        if (_content[_i] == '}')
            break ;
        type = _getDirectiveType();
        _skipWhiteSpaces();
        parameter = _getParameter();
        not_empty = true;
        switch (type) {

        case ALLOWED_METHODS:
            handleAllowedMethods(parameter, location);
            break;
        case REDIRECTION:
            handleRedirection(parameter, location);
            break;
        case ALIAS:
            handleAlias(parameter, location, server_block);
            break;
        case AUTOINDEX:
            handleAutoIndex(parameter, location);
            break;
        case INDEX:
            handleIndex(parameter, location, server_block);
            break;
        case UPLOAD:
            handleUpload(parameter, location, server_block);
            break;
        case CGI:
            handleCgi(parameter, location);
            break;
        default:
            Logger::log(RED, ERROR, "Config file misconfigured: invalid directive in location");
            exit(EXIT_FAILURE);
        }
    }
    if (_content[_i] != '}')
    {
        Logger::log(RED, ERROR, "Config file misconfigured: missing '}'");
        exit(EXIT_FAILURE);
    }
    _i++;
    if (not_empty)
        server_block._locations.insert(std::pair<std::string, Location>(path, location));
}

/*
gets the next directive in the _content string and sets the setting to the corresponding server_block
*/
void    ConfigParser::_getDirective(ServerBlock &server_block)
{
    Directive       type;
    std::string     parameter;

    _skipWhiteSpaces();
    type = _getDirectiveType();
    _skipWhiteSpaces();
    if (type != UNKNOWN && type != LOCATION)
        parameter = _getParameter();
    switch (type) {

    case ROOT:
        handleRoot(parameter, server_block);
        break;
    case LISTEN:
        handleListen(parameter, server_block);
        break;
    case SERVER_NAME:
        handleServerNames(parameter, server_block);
        break;
    case CLIENT_MAX_BODY_SIZE:
        handleClientMaxBodySize(parameter, server_block);
        break;
    case ERROR_PAGE:
        handleErrorPage(parameter, server_block);
        break;
    case LOCATION:
        _getLocation(server_block);
        break;
    default:
        Logger::log(RED, ERROR, "Config file misconfigured: invalid directive in server block");
        exit(EXIT_FAILURE);
    }
}

/*
initialize the DEFAULT valeues for ServerBlock struct
*/
void    ConfigParser::_setDefaultValues(ServerBlock &server_block)
{
    server_block._ip = DEFAULT_HOST;
    try
    {
        server_block._host = ipStringToNumeric(DEFAULT_HOST);
    }
    catch(const std::exception& e)
    {
        Logger::log(RED, ERROR, "Webserv header misconfigured: DEFAULT_HOST: ip invalid: %s", e.what());
        exit(EXIT_FAILURE);
    }
    server_block._port = DEFAULT_PORT;
    server_block._root = DEFAULT_ROOT;
    server_block._client_max_body_size = DEFAULT_CLIENT_MAX_BODY_SIZE;
    server_block._socket = NULL;
}

// ==========   Member functions   =========== //
/*
Parses the config file and adds every server block with the corresponding settings to the _server_blocks vector
*/
void    ConfigParser::parse(std::string config)
{
    _readConfig(config);

    for (; _i < _content.length();)
    {
        ServerBlock server_block;

        _setDefaultValues(server_block);
        _findNextServerBlock();
        while (_i < _content.length())
        {
            _getDirective(server_block);
            _skipWhiteSpaces();
            if (_content[_i] == '}')
                break;
        }
        if (_content[_i] != '}')
        {
            Logger::log(RED, ERROR, "Config file misconfigured: missing '}'");
            exit(EXIT_FAILURE);
        }
        _i++;
        _server_blocks.push_back(server_block);
        _skipWhiteSpaces();
    }
}
