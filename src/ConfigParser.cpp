#include "../inc/ServerManager.hpp"

ConfigParser::ConfigParser(std::vector<Server> &servers) : _servers(servers)
{
}

static std::string    readConfig(std::string config)
{
    std::ifstream       file(config);
    std::stringstream   buffer;

    if (file.fail())
    {
        std::cerr << "Error: Unable to open file " << config << std::endl;
        exit(EXIT_FAILURE);
    }
    buffer << file.rdbuf();
    file.close();
    return (buffer.str());
}

static int skipComment(std::string &content, int i)
{
    if (content[i] == '#')
    {
        while (i < content.length() && content[i] != '\n')
            i++;
        if (content[i] == '/n')
            i++;
        return (i);
    }
    return (i);
}

static int skipWhiteSpaces(std::string &content, int i)
{
    for (; i < content.length(); i++)
    {
        i = skipComment(content, i);
        if (!isspace(content[i]))
            return (i);
    }
    return (i);
}

static int findServerBlock(std::string &content, int i)
{
    for (; i < content.length(); i++)
    {
        i = skipWhiteSpaces(content, i);
        if (content.compare(i, 6, "Server") == 0)
        {
            i += 6;
            i = skipWhiteSpaces(content, i);
            if (content[i] == '{')
                return (i + 1);
        }
        else
        {
            std::cerr << "Error: Config file misconfigured" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    return i;
}

static Directive getDirectiveType(std::string &content, int &i)
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

        if (content.compare(i, keyword.length(), keyword) == 0)
        {
            i += keyword.length();
            if (content[i] == ' ')
                return (responseCode);
            else
                break ;
        }
    }
    return (UNKNOWN);
}

static std::string getParameter(std::string &content, int &i)
{
    std::string parameter;
    int         start;

    start = i;
    for (; i < content.length(); i++)
    {
        if (content[i] == ';')
        {
            i++;
            break;
        }
        if (i == content.length() || content[i] == '\n')
        {
            std::cerr << "Error: Config file misconfigured: missing ';'" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    parameter = content.substr(start, i);
    return (parameter);
}

static int getDirective(std::string &content, int i, Server &server)
{
    std::string parameter;
    Directive   type;

    i = skipWhiteSpaces(content, i);
    type = getDirectiveType(content, i);
    i = skipWhiteSpaces(content, i);
    parameter = getParameter(content, i);
    switch (type)
    {
        case ROOT:
            server.setRoot(parameter);
            break;
        case LISTEN:
            server.setListen(parameter);
            break;
        case SERVER_NAME:
            server.setServerName(parameter);
            break;
        case CLIENT_MAX_BODY_SIZE:
            server.setClientMaxBodySize(parameter);
            break;
        case ERROR_PAGE:
            server.setErrorPage(parameter);
            break;
        // case LOCATION:
        //     setLocation(content, i);
        //     break;
        case UNKNOWN:
            std::cerr << "Error: Config file misconfigured: wrong Directive" << std::endl;
            exit(EXIT_FAILURE);
    }
}

void    ConfigParser::parse(std::string config)
{
    std::string content(readConfig(config));
    Server      server;

    for(size_t i = 0; i < content.length(); i++)
    {
        i = findServerBlock(content, i);
        for (;i < content.length(); i++)
        {
            if (content[i] == '}')
            {
                i++;
                break ;
            }
            i = getDirective(content, i, server);
        }
        _servers.push_back(server);
        server.clear();
    }
}
