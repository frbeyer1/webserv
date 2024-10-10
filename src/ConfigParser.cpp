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
*/
static uint32_t ipStringToNumeric(const std::string& ip)
{
    std::stringstream   ss(ip);
    std::string         segment;
    uint32_t            numericIp = 0;
    int                 segmentCount = 0;

    while (std::getline(ss, segment, '.'))
    {
        if (segmentCount >= 4)
            return (-1);
        int segmentValue = std::atoi(segment.c_str());
        if (segmentValue < 0 || segmentValue > 255)
            return (-1);
        numericIp = (numericIp << 8) | segmentValue;
        segmentCount++;
    }
    if (segmentCount != 4)
        return (-1);
    return (numericIp);
}

/*
parses an parameter string of the config and sets root on the corresponding server
*/
static void    handleRoot(std::string parameter, Server &server)
{

}

/*
parses an parameter string of the config and sets port and host on the corresponding server
*/
static void    handleListen(std::string parameter, Server &server)
{
    std::string port_str;
    std::string ip_str;
    bool        found_host = false;
    in_addr_t   host;
    uint16_t    port;

    for (int i = 0; i < parameter.length(); i++)
    {
        if (parameter[i] == ':')
        {
            found_host = true;
            if (parameter.compare(0, i, "default:") == 0)
                ip_str = DEFAULT_HOST;
            else
                ip_str = parameter.substr(0, i - 1);
            port_str = parameter.substr(i + 1, parameter.length());
            break ;
        }
    }
    if (!found_host)
    {
        port_str = parameter;
        ip_str = DEFAULT_HOST;
    }
    for (int i = 0; i < ip_str.length(); i++)
    {
        if (!isdigit(ip_str[i]) && ip_str[i] != '.')
        {
            Logger::log(RED, ERROR, "Config file misconfigured: listen directive: ip invalid");
            exit(EXIT_FAILURE);
        }
    }
    if (host = ipStringToNumeric(ip_str) < 0)
    {
        Logger::log(RED, ERROR, "Config file misconfigured: listen directive: ip invalid");
        exit(EXIT_FAILURE);
    }
    server.setHost(host);
    for (int i = 0; i < port_str.length(); i++)
    {
        if (!isdigit(port_str[i]))
        {
            Logger::log(RED, ERROR, "Config file misconfigured: listen directive: port invalid");
            exit(EXIT_FAILURE);
        }
    }
    port = atoi(port_str.c_str());
    server.setPort(port);
}

/*
parses an parameter string of the config and sets the server_name on the corresponding server
*/
static void    handleServerName(std::string parameter, Server &server)
{

}

/*
parses an parameter string of the config and sets the client_max_body_size on the corresponding server
*/
static void    handleClientMaxBodySize(std::string parameter, Server &server)
{

}

/*
parses an parameter string of the config and sets custom error pages on the corresponding server
*/
static void    handleErrorPage(std::string parameter, Server &server)
{

}

// ======   Private member functions   ======= //
/*
reads and saves the config file
*/
void    ConfigParser::_readConfig(std::string config)
{
    std::ifstream       file(config);
    std::stringstream   buffer;

    if (file.fail())
    {
        std::string msg("Unable to open file: " + config);
        Logger::log(RED, ERROR, msg.c_str());
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
        if (_content[_i] == '/n')
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
        if (_content.compare(_i, 6, "Server") == 0)
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
            Logger::log(RED, ERROR, "Config file misconfigured: found something else than server block");
            exit(EXIT_FAILURE);
        }
    }
}

/*
returns Directive type
*/
Directive ConfigParser::_getDirectiveType()
{
    const std::map<std::string, Directive> keywordMap = {
        {"root", ROOT},
        {"listen", LISTEN},
        {"server_name", SERVER_NAME},
        {"client_max_body_size", CLIENT_MAX_BODY_SIZE},
        {"error_page", ERROR_PAGE},
        {"allowed_methods", ALLOWED_METHODS},
        {"redirection", REDIRECTION},
        {"alias", ALIAS},
        {"autoindex", AUTOINDEX},
        {"index", INDEX},
        // ...
        {"location", LOCATION},
    };
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
            _i++;
            break;
        }
        if (_i == _content.length() || _content[_i] == '\n')
        {
            Logger::log(RED, ERROR, "Config file misconfigured: missing ';'");
            exit(EXIT_FAILURE);
        }
    }
    parameter = _content.substr(start, _i);
    return (parameter);
}

void    ConfigParser::_getLocation(Server &server)
{
    Directive   type;
    location_t  location;

    // type = getDirectiveType()
    switch (type)
    {
        case ALLOWED_METHODS:
            // handleAllowedMethods(parameter, location);
            break;
        case REDIRECTION:
            // handleRedirection(parameter, location);
            break;
        case ALIAS:
            // handleAlias(parameter, location);
            break;
        case AUTOINDEX:
            // handleAutoIndex(parameter, location);
            break;
        case INDEX:
            // handleIndex(parameter, location);
            break;
        // case ...:
        default:
            Logger::log(RED, ERROR, "Config file misconfigured: invalid directive");
            exit(EXIT_FAILURE);
    }
    server.setLocation(location);
}

/*
parses the directive dependent of the directive type
*/
void    ConfigParser::_getDirective(Server &server)
{
    std::string parameter;
    Directive   type;

    _skipWhiteSpaces();
    type = _getDirectiveType();
    _skipWhiteSpaces();
    parameter = _getParameter();
    switch (type)
    {
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
            Logger::log(RED, ERROR, "Config file misconfigured: invalid directive");
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
        for (;_i < _content.length(); _i++)
        {
            if (_content[_i] == '}')
            {
                _i++;
                break ;
            }
            _getDirective(server);
        }
        _servers.push_back(server);
    }
}
