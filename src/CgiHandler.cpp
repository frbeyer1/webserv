#include "../inc/CgiHandler.hpp"

// =============   Constructor   ============= //
CgiHandler::CgiHandler(Request &request, ServerBlock &server, std::string script_path, std::string binary_path, sockaddr_in client_addr) : _request(request), _server(server)
{
    _state = CGI_HEADER_START;
    _script_path = script_path;
    _binary_path = binary_path;
    _body = "";
    _error = OK;
    _client_addr = client_addr;
}

// ============   Deconstructor   ============ //
CgiHandler::~CgiHandler()
{
    if (_env != NULL)
	{
		for (size_t i = 0; _env[i]; i++)
			delete[] _env[i];
		delete[] _env;
	}
}

// ==============   Getters   ================ //
int CgiHandler::getError() const
{
    return _error;
}

const std::map<std::string, std::string>& CgiHandler::getHeaders() const
{
    return _headers;
}

std::string CgiHandler::getBody() const
{
    return _body;
}

// ======   Private member functions   ======= //
/*
checks if the header from the cgi output is vaild and inserts it into the header map
    - returns true if valid and false if not
*/
bool CgiHandler::_addHeader(std::string &header_name, std::string &header_value)
{
    std::vector<std::string> valid_headers;

    valid_headers.push_back("Accept-Charset");
    valid_headers.push_back("Accept-Ranges");
    valid_headers.push_back("Age");
    valid_headers.push_back("Allow");
    valid_headers.push_back("Cache-Control");
    valid_headers.push_back("Connection");
    valid_headers.push_back("Content-Disposition");
    valid_headers.push_back("Content-Encoding");
    valid_headers.push_back("Content-Language");
    valid_headers.push_back("Content-Length");
    valid_headers.push_back("Content-Location");
    valid_headers.push_back("Content-Range");
    valid_headers.push_back("Content-Type");
    valid_headers.push_back("Date");
    valid_headers.push_back("ETag");
    valid_headers.push_back("ETa");
    valid_headers.push_back("Expires");
    valid_headers.push_back("Last-Modified");
    valid_headers.push_back("Link");
    valid_headers.push_back("Location");
    valid_headers.push_back("Pragma");
    valid_headers.push_back("Proxy-Authenticate");
    valid_headers.push_back("Retry-After");
    valid_headers.push_back("Server");
    valid_headers.push_back("Set-Cookie");
    valid_headers.push_back("Status");
    valid_headers.push_back("Strict-Transport-Security");
    valid_headers.push_back("Transfer-Encoding");
    valid_headers.push_back("Vary");
    valid_headers.push_back("WWW-Authenticate");
    valid_headers.push_back("X-Content-Type-Options");
    valid_headers.push_back("X-Frame-Options");
    valid_headers.push_back("X-Requested-With");

    if (find(valid_headers.begin(), valid_headers.end(), header_name) != valid_headers.end())
    {
	    _headers.insert(std::pair<std::string, std::string>(header_name, header_value));
        return true;
    }
    else
        return false;
}

/*
extracts the cgi output into the headers and the body
    - changes the _error to what is in the Status header if given
*/
void CgiHandler::_parseCgi(std::string &output)
{
    uint8_t         ch = 0;
	std::string		header_name = "";
	std::string		header_value = "";
    bool            parsing_finished = false;

    size_t i = 0;
    for (; i < output.size() && !parsing_finished; i++)
	{
		ch = output[i];

		switch (_state) {

		case CGI_HEADER_START:
			if (ch == CR)
			{
				_state = CGI_PARSING_FINISHED;
				break;
			}
			else if (ch == LF)
			{
				parsing_finished = true;
				break;
			}
			else
				_state = CGI_HEADER_KEY;
				// fall through
		case CGI_HEADER_KEY: 
			if (ch == ':')
			{
				_state = CGI_HEADER_WS;
				break;
			}
			else
			{
				header_name.append(1, ch);
				break;
			}
		case CGI_HEADER_WS:
			if (iswspace(ch))
            {
				_state = CGI_HEADER_VALUE;
				break;
            }
			else
            {
                if (!_headers.empty())
                {
                    Logger::log(RED, ERROR, "Invalid HTTP response format: Expected space after the header name. Check CGI script: %s", _script_path.c_str());
                    _error = INTERNAL_SERVER_ERROR;
                    return;
                }
                else
                {
                    _body = output;
                    parsing_finished = true;
                    break;
                }
            }
		case CGI_HEADER_VALUE:
			if (ch == CR)
				break;
			if (ch == LF)
				_state = CGI_HEADER_END;
			else
			{
				header_value.append(1, ch);
				break;
			}
			// fall through
		case CGI_HEADER_END:
            if (!_addHeader(header_name, header_value))
            {
                _error = INTERNAL_SERVER_ERROR;
                Logger::log(RED, ERROR, "Invalid HTTP response format: Invalid HTTP header. Check CGI script: %s", _script_path.c_str());
            }
			header_name.clear();
			header_value.clear();
			_state = CGI_HEADER_START;
			break;
		case CGI_PARSING_FINISHED:
			if (ch == LF)
			{
				parsing_finished = true;
				break;
			}
			_error = INTERNAL_SERVER_ERROR;
            Logger::log(RED, ERROR, "Invalid HTTP response format: Expected newline character ('\n') after carriage return ('\r'). Check CGI script: %s", _script_path.c_str());
            return;
		}
    }

    // extract body
    if (_body.empty())
    {
        if (output[i])
            _body = output.substr(i, output.size() - i);
        else
            _body = output;
    }

    // check for status header
    if (_headers.count("Status"))
    {
        _error = atoi(_headers["Status"].c_str());
        if (_error < 100 || _error >= 600)
        {
            _error = INTERNAL_SERVER_ERROR;
            Logger::log(RED, ERROR, "Invalid HTTP status code specified in CGI script");
            return;
        }
    }
}

/*
reads cgi output from fd
*/
void CgiHandler::_readCgi(int fd) 
{
    const int BUFSIZE = 4096;
	char buffer[BUFSIZE];
	int bytesRead = 1;
	std::string output;

	while (bytesRead > 0) 
    {		
		bytesRead = read(fd, buffer, BUFSIZE);
		if (bytesRead < 0)
        {
            _error = INTERNAL_SERVER_ERROR;
            Logger::log(RED, ERROR, "CGI: Read error on fd[%i]: %s", fd, strerror(errno));
            return;
        }
		output.append(buffer, bytesRead);
	}
    _parseCgi(output);
}

/*
building the environment for the cgi call
*/
void CgiHandler::_buildEnvironment()
{
    std::map<std::string, std::string> tmp_env;

    // build temporary environment
    if (_request.getMethod() == POST)
    {
        tmp_env["CONTENT_LENGTH"] = intToStr(_request.getBody().size());
        tmp_env["CONTENT_TYPE"] = const_cast<std::map<std::string, std::string>&>(_request.getHeaders())["Content-Type"];
    }
    tmp_env["AUTH_TYPE"] = "";
    tmp_env["PATH_INFO"] = _request.getPath(); // must be full path (requested by subject)
    tmp_env["PATH_TRANSLATED"] = _server._root + _request.getPath();
    tmp_env["QUERY_STRING"] = _request.getQuery();
    tmp_env["REMOTE_ADDR"] = inAddrToIpString(_client_addr.sin_addr.s_addr);
    // tmp_env["REMOTE_HOST"] = ""; // not needed
    // tmp_env["REMOTE_IDENT"] = ""; // not needed
    // tmp_env["REMOTE_USER"] = ""; // not needed
    tmp_env["REQUEST_METHOD"] = _request.getMethodStr();
    tmp_env["SCRIPT_NAME"] = _script_path;
    tmp_env["SERVER_NAME"] = _server._ip; // takes the first server name
    tmp_env["SERVER_PORT"] = intToStr(_server._port);
    tmp_env["SERVER_PROTOCOL"] = "HTTP/1.1";
    tmp_env["SERVER_SOFTWARE"] = "Webserv";
    tmp_env["REDIRECT_STATUS"] = intToStr(_request.getError());

    // add request headers to env
	for(std::map<std::string, std::string>::const_iterator it = _request.getHeaders().begin(); it != _request.getHeaders().end(); it++)
	{
		std::string name = "HTTP_" + it->first;

        for (size_t i = 0; i < name.size(); i++)
        {
            // convert to upper case
            name[i] = std::toupper(name[i]);
            // replaced "-" with "_"
            if (name[i] == '-')
                name[i] = '_';
        }
		tmp_env[name] = it->second;
	}

    // builds environemnt as char** for the execve call
    _env = new char*[tmp_env.size() + 1];
    size_t i = 0;
    for (std::map<std::string, std::string>::iterator it = tmp_env.begin(); it != tmp_env.end(); it++)
	{
		_env[i] = new char[it->first.size() + it->second.size() + 2];
        std::string line = it->first + "=" + it->second;
		std::strcpy(_env[i], line.c_str());
        i++;
	}
	_env[tmp_env.size()] = NULL;
}

// ==========   Member functions   =========== //
/*
executes cgi
*/
void CgiHandler::execCgi() 
{
    try
    {
        _buildEnvironment();
    }
    catch(const std::exception& e)
    {
        Logger::log(RED, ERROR, "Failed to build the environment for CGI: %e", e.what());
        _error = INTERNAL_SERVER_ERROR;
        return ;
    }
    
    char *argv[3];
    argv[0] = const_cast<char*>(_binary_path.c_str());
    argv[1] = const_cast<char*>(_script_path.c_str());
    argv[2] = NULL;

    int cgifd[2];
    if (!_request.getBody().empty())
    {
        if (pipe(cgifd) == -1)
        {
            Logger::log(RED, ERROR, "Creating cgi pipe has failed, aborting CGI init process.");
            return;
        }
        write(cgifd[1], _request.getBody().c_str(), _request.getBody().length());
        close(cgifd[1]);
    }
    else 
    {
        cgifd[0] = 0;
        cgifd[1] = 1;
    }
    int fd[2], pid, status;
    if (pipe(fd) == -1)
    {
        Logger::log(RED, ERROR, "Creating pipe has failed, aborting CGI init process.");
        _error = INTERNAL_SERVER_ERROR;
        return;
    }
    if ((pid = fork()) == -1)
    {
        Logger::log(RED, ERROR, "Creating fork has failed, aborting CGI init process.");
        _error = INTERNAL_SERVER_ERROR;
        return;
    }
    if (!pid)
    {
        if (dup2(fd[1], 1) == -1)
        {
            Logger::log(RED, ERROR, "Child Process ID: %i: dup2 has failed (WRITE)", pid);
            _error = INTERNAL_SERVER_ERROR;
            exit(EXIT_FAILURE);
        }
        if (cgifd[0] != 0 && dup2(cgifd[0], 0) == -1) //if cgifd is not unset, set it now
        { 
            Logger::log(RED, ERROR, "Child Process ID: %i: dup2 has failed (READ)", pid);
            _error = INTERNAL_SERVER_ERROR;
            exit(EXIT_FAILURE);
        }
        execve(*argv, argv, _env);
        Logger::log(RED, ERROR, "Child Process ID: %i: Execve has failed", pid);
        _error = INTERNAL_SERVER_ERROR;
        exit(EXIT_FAILURE);
    }
    waitpid(pid, &status, 0);
    if (!WIFEXITED(status))
    {
        _error = INTERNAL_SERVER_ERROR;
        // close(fd[0]) ????
        // close(fd[1]) ????
        // close(cgifd[0]) ????
        return;
    }
    close(fd[1]); //WRITE END
    _readCgi(fd[0]);
    if (!_request.getBody().empty())
        close(cgifd[0]);
    close(fd[0]); //READ END
    return;
}
