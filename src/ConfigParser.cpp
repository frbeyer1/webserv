#include "../inc/ServerManager.hpp"

// =============   Constructor   ============= //
ConfigParser::ConfigParser(std::vector<Server> &servers) : _servers(servers)
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
    return (numericIp);
}

/*
parses an parameter string of the config and sets root on the corresponding server
*/
static void handleRoot(std::string parameter, Server &server)
{
    struct stat buf;

    if (stat(parameter.c_str(), &buf) != 0)
    {
        Logger::log(RED, ERROR, std::ostringstream() << "Error: Config file misconfigured: root directive: path invalid");
        exit(EXIT_FAILURE);
    }
    if (S_ISDIR(buf.st_mode))
        server.setRoot(parameter);
    else
    {
        Logger::log(RED, ERROR, std::ostringstream() << "Config file misconfigured: root directive: is no directory");
        exit(EXIT_FAILURE);
    }
}

/*
parses an parameter string of the config and sets port and host on the corresponding server
*/
static void handleListen(std::string parameter, Server &server)
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
            if (parameter.compare(0, i, "localhost:") == 0)
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
            Logger::log(RED, ERROR, std::ostringstream() << "Config file misconfigured: listen directive: IP invalid");
            exit(EXIT_FAILURE);
        }
    }
    try
    {
        host = ipStringToNumeric(ip_str);
    }
    catch(const std::exception& e)
    {
        Logger::log(RED, ERROR, std::ostringstream() << "Config file misconfigured: listen directive: IP invalid: " << e.what());
        exit(EXIT_FAILURE);
    }
    server.setHost(host);
    server.setIp(ip_str);
    for (size_t i = 0; i < port_str.length(); i++)
    {
        if (!isdigit(port_str[i]))
        {
            Logger::log(RED, ERROR, std::ostringstream() << "Config file misconfigured: listen directive: port invalid");
            exit(EXIT_FAILURE);
        }
    }
    port = atoi(port_str.c_str());
    if (port < 1 || port > 65636)
    {
        Logger::log(RED, ERROR, std::ostringstream() << "Config file misconfigured: listen directive: port invalid");
        exit(EXIT_FAILURE);
    }
    server.setPort(port);
}

/*
parses an parameter string of the config and sets the server_name on the corresponding server
valid characters:
    - Letters (a-z, A-Z)
    - Digits (0-9)
    - Hyphens (-)
    - Periods (.)
    - Tildes (~)
    - Underscores (_)
*/
static void handleServerName(std::string parameter, Server &server)
{
    for (size_t i = 0; i < parameter.length(); i++)
    {
        if ((parameter[i] < 'a' || parameter[i] > 'z') && (parameter[i] < 'A' || parameter[i] > 'Z') 
            && (parameter[i] < '0' || parameter[i] > '9') && parameter[i] != '.' && parameter[i] != '-' && parameter[i] != '~' && parameter[i] != '_' )
        {
            Logger::log(RED, ERROR, std::ostringstream() << "Config file misconfigured: server_name directive: invalid character");
            exit(EXIT_FAILURE);
        }
    }
    server.setServerName(parameter);
}

/*
parses an parameter string of the config and sets the client_max_body_size on the corresponding server
*/
static void handleClientMaxBodySize(std::string parameter, Server &server)
{
    size_t  size;

    for (size_t i = 0; i < parameter.length(); i ++)
    {
        if (!isdigit(parameter[i]))
        {
            Logger::log(RED, ERROR, std::ostringstream() << "Config file misconfigured: client_max_body_size directive: invalid character");
            exit(EXIT_FAILURE);
        }
    }
    size = atoi(parameter.c_str());
    server.setClientMaxBodySize(size);
}

/*
parses an parameter string of the config and sets custom error pages on the corresponding server
*/
static void handleErrorPage(std::string parameter, Server &server)
{
    int         status_code;
    std::string status_code_str;
    std::string page_path;
    struct stat buf;
    size_t      i = 0;

    for (; i < parameter.length(); i++)
    {
        if (i == 3)
            break ;
        if (isdigit(parameter[i]))
            status_code_str.push_back(parameter[i]);
        else
        {
            Logger::log(RED, ERROR, std::ostringstream() << "Config file misconfigured: error_page directive: status code invalid");
            exit(EXIT_FAILURE);
        }
    }
    status_code = atoi(status_code_str.c_str());
    if (i != 3 || status_code < 100 || status_code > 599)
    {
        Logger::log(RED, ERROR, std::ostringstream() << "Config file misconfigured: error_page directive: status code invalid");
        exit(EXIT_FAILURE);
    }
    for (; i < parameter.length(); i++)
    {
        if (i == 3)
        {
            if (!isspace(parameter[i]))
            {
                Logger::log(RED, ERROR, std::ostringstream() << "Config file misconfigured: error_page directive: missing space");
                exit(EXIT_FAILURE);
            }
        }
        page_path.push_back(parameter[i]);
    }
    if (stat(page_path.c_str(), &buf) != 0 || S_ISREG(buf.st_mode) == 0)
    {
        Logger::log(RED, ERROR, std::ostringstream() << "Config file misconfigured: error_page directive: error page path invalid");
        exit(EXIT_FAILURE);
    }
    server.setErrorPage(status_code, page_path);
}

// static void handleAllowedMethods(std::string parameter, location_t &location)
// {

// }

// static void handleRedirection(std::string parameter, location_t &location)
// {

// }

// static void handleAlias(std::string parameter, location_t &location)
// {

// }

static void handleAutoIndex(std::string parameter, location_t &location)
{
    if (parameter == "on")
        location.autoindex = true;
    else if (parameter == "off")
        location.autoindex = false;
    else
    {
        Logger::log(RED, ERROR, std::ostringstream() << "Config file misconfigured: autoindex directive: invalid parameter");
        exit(EXIT_FAILURE);
    }
}

// static void handleIndex(std::string parameter, location_t &location)
// {

// }

// ======   Private member functions   ======= //
/*
reads and saves the config file
*/
void    ConfigParser::_readConfig(std::string config)
{
    std::ifstream       file(config.c_str());
    std::stringstream   buffer;

    if (file.fail())
    {
        Logger::log(RED, ERROR, std::ostringstream() << "Unable to open file: " << config);
        exit(EXIT_FAILURE);
    }
    buffer << file.rdbuf();
    file.close();
    _content = buffer.str();
}

/*
skipping comments in the config file string
*/
void    ConfigParser::_skipComment()
{
    if (_content[_i] == '#')
    {
        while (_i < _content.length() && _content[_i] != '\n')
            _i++;
        if (_content[_i] == '\n')
            _i++;
        return ;
    }
}

/*
skipping whitespaces in the config file string
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
finds next server block in the config file string
*/
void    ConfigParser::_findServerBlock()
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
                return;
            }
        }
        else
        {
            Logger::log(RED, ERROR, std::ostringstream() << "Config file misconfigured: found something else than server block");
            exit(EXIT_FAILURE);
        }
    }
}

/*
returns Directive type
*/
Directive ConfigParser::_getDirectiveType()
{
    std::map<std::string, Directive> keywordMap;
    keywordMap["root"] = ROOT;
    keywordMap["listen"] = LISTEN;
    keywordMap["server_name"] = SERVER_NAME;
    keywordMap["client_max_body_size"] = CLIENT_MAX_BODY_SIZE;
    keywordMap["error_page"] = ERROR_PAGE;
    keywordMap["allowed_methods"] = ALLOWED_METHODS;
    keywordMap["redirection"] = REDIRECTION;
    keywordMap["alias"] = ALIAS;
    keywordMap["autoindex"] = AUTOINDEX;
    keywordMap["index"] = INDEX;
    keywordMap["location"] = LOCATION;
    // ...
    for (std::map<std::string, Directive>::const_iterator it = keywordMap.begin(); it != keywordMap.end(); it++)
    {
        const std::string&  keyword = it->first;
        Directive           responseCode = it->second;

        if (_content.compare(_i, keyword.length(), keyword) == 0)
        {
            _i += keyword.length();
            if (_content[_i] == ' ')
                return (responseCode);
            else
                break ;
        }
    }
    return (UNKNOWN);
}

/*
returns the parameter of an directive as a string
*/
std::string ConfigParser::_getParameter()
{
    std::string parameter;
    int         start;

    start = _i;
    for (; _i < _content.length(); _i++)
    {
        if (_content[_i] == ';')
        {
            if (iswspace(_content[_i - 1]))
            {
                Logger::log(RED, ERROR, std::ostringstream() << "Config file misconfigured: invalid syntax: found whitespace before ';'");
                exit(EXIT_FAILURE);
            }
            _i++;
            break;
        }
        if (_i == _content.length() || _content[_i] == '\n')
        {
            Logger::log(RED, ERROR, std::ostringstream() << "Config file misconfigured: missing ';'");
            exit(EXIT_FAILURE);
        }
    }
    parameter = _content.substr(start, _i - start - 1);
    return (parameter);
}

/*
parses the location directive
*/
// void    ConfigParser::_getLocation(Server &server)
// {
//     Directive   type;
//     location_t  location;
    
//     memset(&location, 0, sizeof(location));
//     // save path
//     for (int i = 0; i < _content.length(); _i++)
//     {

//     }
//     for (int i = 0; i < _content.length(); _i++)
//     {
//         std::string parameter;
//         if (_content[_i] == '}')
//         {
//             _i++;
//             break ;
//         }
//         _skipWhiteSpaces();
//         type = _getDirectiveType();
//         _skipWhiteSpaces();
//         parameter = _getParameter();
//         switch (type) {

//         case ALLOWED_METHODS:
//             handleAllowedMethods(parameter, location);
//             break;
//         case REDIRECTION:
//             handleRedirection(parameter, location);
//             break;
//         case ALIAS:
//             handleAlias(parameter, location);
//             break;
//         case AUTOINDEX:
//             handleAutoIndex(parameter, location);
//             break;
//         case INDEX:
//             handleIndex(parameter, location);
//             break;
//         // case ...:
//         default:
//             Logger::log(RED, ERROR, "Config file misconfigured: invalid directive");
//             exit(EXIT_FAILURE);
//         }
//     }
//     server.setLocation(location);
// }

/*
parses the directive dependent of the directive type
*/
void    ConfigParser::_getDirective(Server &server)
{
    std::string parameter;
    Directive   type;

    type = _getDirectiveType();
    _skipWhiteSpaces();
    if (type != LOCATION && type != UNKNOWN)
        parameter = _getParameter();
    switch (type) {
    
    case ROOT:
        handleRoot(parameter, server);
        break;
    case LISTEN:
        handleListen(parameter, server);
        break;
    case SERVER_NAME:
        handleServerName(parameter, server);
        break;
    case CLIENT_MAX_BODY_SIZE:
        handleClientMaxBodySize(parameter, server);
        break;
    case ERROR_PAGE:
        handleErrorPage(parameter, server);
        break;
    case LOCATION:
        _getLocation(server);
        break;
    default:
        Logger::log(RED, ERROR, std::ostringstream() << "Config file misconfigured: invalid directive");
        exit(EXIT_FAILURE);
    }
}

// ==========   Member functions   =========== //
/*
parses the config file and adds the servers with the right settigs to the server_vector 
*/
void    ConfigParser::parse(std::string config)
{
    _readConfig(config);

    for(;_i < _content.length(); _i++)
    {
        Server      server;

        _findServerBlock();
        _skipWhiteSpaces();
        for (;_i < _content.length();)
        {
            _getDirective(server);
            _skipWhiteSpaces();
            if (_content[_i] == '}')
                break ;
        }
        if (_content[_i] != '}')
        {
            Logger::log(RED, ERROR, std::ostringstream() << "Config file misconfigured: missing '}'");
            exit(EXIT_FAILURE);
        }
        else
            _i++;
        _servers.push_back(server);
    }
}
