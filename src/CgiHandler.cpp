#include "../inc/CgiHandler.hpp"

// =============   Constructor   ============= //

CgiHandler::CgiHandler()
{
    _state = Cgi_Header_Start;
    _body = "";
    _error = OK;
    _cgi_pid = 0;
    _start_time = 0;
    pipe_in[0] = -1;
    pipe_in[1] = -1;
    pipe_out[0] = -1;
    pipe_out[1] = -1;
    finished_execution = false;
    _env = NULL;
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

int CgiHandler::getCgiPid() const
{
    return _cgi_pid;
}

time_t CgiHandler::getStartTime() const
{
    return _start_time;
}

// ==============   Setters   ================ //

void CgiHandler::setError(int error)
{
    _error = error;
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
building the environment for the cgi call
*/
void CgiHandler::_buildEnvironment(Request &request, ServerBlock &server, sockaddr_in client_addr)
{
    std::map<std::string, std::string> tmp_env;

    // build temporary environment
    if (request.getMethod() == POST)
    {
        tmp_env["CONTENT_LENGTH"] = intToStr(request.getBody().size());
        tmp_env["CONTENT_TYPE"] = const_cast<std::map<std::string, std::string>&>(request.getHeaders())["Content-Type"];
    }
    tmp_env["AUTH_TYPE"] = "";
    tmp_env["PATH_INFO"] = request.getPath(); // must be full path (requested by subject)
    tmp_env["PATH_TRANSLATED"] = server.root + request.getPath();
    tmp_env["QUERY_STRING"] = request.getQuery();
    tmp_env["REMOTE_ADDR"] = inAddrToIpString(client_addr.sin_addr.s_addr);
    // tmp_env["REMOTE_HOST"] = ""; // not needed
    // tmp_env["REMOTE_IDENT"] = ""; // not needed
    // tmp_env["REMOTE_USER"] = ""; // not needed
    tmp_env["REQUEST_METHOD"] = request.getMethodStr();
    tmp_env["SCRIPT_NAME"] = _script_path;
    tmp_env["SERVER_NAME"] = server.ip; // takes the first server name
    tmp_env["SERVER_PORT"] = intToStr(server.port);
    tmp_env["SERVER_PROTOCOL"] = "HTTP/1.1";
    tmp_env["SERVER_SOFTWARE"] = "Webserv";
    tmp_env["REDIRECT_STATUS"] = intToStr(request.getError());

    // add request headers to env
	for(std::map<std::string, std::string>::const_iterator it = request.getHeaders().begin(); it != request.getHeaders().end(); it++)
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

    try
    {
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
    catch(const std::exception& e)
    {
        Logger::log(RED, ERROR, "Failed to build the environment for CGI: %e", e.what());
        _error = INTERNAL_SERVER_ERROR;
        return ;
    }
}

// ==========   Member functions   =========== //

/*
extracts the cgi output into the headers and the body
    - changes the _error to what is in the Status header if given
*/
void CgiHandler::parseCgi(uint8_t *data, size_t size)
{
    uint8_t         ch;
	std::string		header_name, header_value;

    if (_error != OK)
        return;
    for (size_t i = 0; i < size; i++)
	{
		ch = data[i];
		switch (_state) {
        
		case Cgi_Header_Start:
			if (ch == CR)
			{
				_state = Cgi_End_LF;
				break;
			}
			else if (ch == LF)
			{
                _state = Cgi_Parsing_Finished;
				break;
			}
			else
				_state = Cgi_Header_Key;
			// fall through
		case Cgi_Header_Key: 
			if (ch == ':')
			{
				_state = Cgi_Header_WS;
				break;
			}
			else
			{
				header_name.push_back(ch);
				break;
			}
		case Cgi_Header_WS:
			if (iswspace(ch))
            {
				_state = Cgi_Header_Value;
				break;
            }
            else
            {
				_state = Cgi_Header_Value;
                _error = INTERNAL_SERVER_ERROR;
                Logger::log(RED, ERROR, "Invalid HTTP response format: Expected space after the header name. Check CGI script: %s", _script_path.c_str());
                return;
            }
			// fall through
		case Cgi_Header_Value:
			if (ch == CR)
				break;
			if (ch == LF)
				_state = Cgi_Header_End;
			else
			{
				header_value.push_back(ch);
				break;
			}
			// fall through
		case Cgi_Header_End:
            // check for status header
            if (header_name == "Status")
            {
                _error = atoi(header_value.c_str());
                if (_error < 100 || _error >= 600)
                {
                    _error = INTERNAL_SERVER_ERROR;
                    Logger::log(RED, ERROR, "Invalid HTTP status code specified in CGI script");
                    return;
                }
            }
            if (!_addHeader(header_name, header_value))
            {
                _error = INTERNAL_SERVER_ERROR;
                Logger::log(RED, ERROR, "Invalid HTTP response format: Invalid HTTP header. Check CGI script: %s", _script_path.c_str());
                return;
            }
			header_name.clear();
			header_value.clear();
			_state = Cgi_Header_Start;
			break;
		case Cgi_End_LF:
			if (ch == LF)
			{
                _state = Cgi_Parsing_Finished;
				break;
			}
			_error = INTERNAL_SERVER_ERROR;
            Logger::log(RED, ERROR, "Invalid HTTP response format: Expected newline character ('\n') after carriage return ('\r'). Check CGI script: %s", _script_path.c_str());
            return;
            break;
        case Cgi_Parsing_Finished:
            // extract body
            _body.push_back(ch);
            break;
		}
    }
}

/*
clear the CGIHandler object
*/
void CgiHandler::clear()
{
    _state = Cgi_Header_Start;
    _body = "";
    _error = OK;
    _script_path = "";
    _binary_path = "";
    _start_time = 0;
    pipe_in[0] = -1;
    pipe_in[1] = -1;
    pipe_out[0] = -1;
    pipe_out[1] = -1;
    finished_execution = false;
    if (_env != NULL)
	{
		for (size_t i = 0; _env[i]; i++)
			delete[] _env[i];
		delete[] _env;
	}
    _env = NULL;
}

/*
close all open fd for pipes
*/
void CgiHandler::_closePipes()
{
    if (pipe_in[0] != -1)
        close(pipe_in[0]);
    if (pipe_in[1] != -1)
        close(pipe_in[1]);
    if (pipe_out[0] != -1)
        close(pipe_out[0]);
    if (pipe_out[1] != -1)
        close(pipe_out[1]);
}

/*
executes cgi
*/
void CgiHandler::execCgi(Request &request, ServerBlock &server, std::string script_path, std::string binary_path, sockaddr_in client_addr) 
{
    _script_path = script_path;
    _binary_path = binary_path;
    try
    {
        _buildEnvironment(request, server, client_addr);
    }
    catch(const std::exception& e)
    {
        Logger::log(RED, ERROR, "Failed to build the environment for CGI: %e", e.what());
        _error = INTERNAL_SERVER_ERROR;
        return ;
    }

    // builds argv for execve
    char *argv[3];
    argv[0] = const_cast<char*>(_binary_path.c_str());
    argv[1] = const_cast<char*>(_script_path.c_str());
    argv[2] = NULL;

    // opens pipe_in if request has body
    if (!request.getBody().empty())
    {
        if (pipe(pipe_in) == -1)
        {
            Logger::log(RED, ERROR, "Creating cgi pipe has failed, aborting CGI init process.");
            return;
        }
        setNonBlocking(pipe_in[1]);
    }

    // opens pipe_out
    if (pipe(pipe_out) == -1)
    {
        _closePipes();
        _error = INTERNAL_SERVER_ERROR;
        Logger::log(RED, ERROR, "Creating pipe has failed, aborting CGI init process.");
        return;
    }
    setNonBlocking(pipe_out[0]);

    // creates child process
    if ((_cgi_pid = fork()) == -1)
    {
        _closePipes();
        _error = INTERNAL_SERVER_ERROR;
        Logger::log(RED, ERROR, "Creating fork has failed, aborting CGI init process.");
        return;
    }

    // child process
    if (_cgi_pid == 0)
    {
        if (dup2(pipe_out[1], 1) == -1)
        {
            _closePipes();
            _error = INTERNAL_SERVER_ERROR;
            Logger::log(RED, ERROR, "Child Process ID: %i: dup2 has failed (WRITE)", _cgi_pid);
            exit(EXIT_FAILURE);
        }
        if (pipe_in[0] != -1 && dup2(pipe_in[0], 0) == -1)
        {
            _closePipes();
            _error = INTERNAL_SERVER_ERROR;
            Logger::log(RED, ERROR, "Child Process ID: %i: dup2 has failed (READ)", _cgi_pid);
            exit(EXIT_FAILURE);
        }
        _closePipes();
        execve(*argv, argv, _env);
        _error = INTERNAL_SERVER_ERROR;
        Logger::log(RED, ERROR, "Child Process ID: %i: Execve has failed", _cgi_pid);
        exit(EXIT_FAILURE);
    }
    else
    {
        _start_time = time(NULL);
        close(pipe_out[1]);
        if (pipe_in[0] != -1)
            close(pipe_in[0]);
    }
}
