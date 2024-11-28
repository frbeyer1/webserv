#include "../inc/Logger.hpp"

LogState Logger::_state = DEFAULT_LOGGER_STATE;
LogLevel Logger::_level = DEFAULT_LOGGER_LEVEL;
LogOutput Logger::_output = DEFAULT_LOGGER_OUTPUT;

// ================   Setter   =============== //
void    Logger::setState(LogState state)
{
    _state = state;
}

void    Logger::setLogLevel(LogLevel level)
{
    _level = level;
}

void    Logger::setOutputMode(LogOutput output)
{
    _output = output;
}

// ================   Utils   ================ //
/*
returns an timestamp with the format [day/month/year  hours:minutes:seconds]
*/
static std::string getTimestamp()
{
    char        buffer[50];
    std::time_t t = std::time(NULL);
    std::tm     tm = *std::localtime(&t);

    std::strftime(buffer, sizeof(buffer), "[%d/%b/%Y  %H:%M:%S]", &tm);
    return (std::string(buffer));
}

/*
returns an string with the loglvl
*/
static std::string getLogLevelStr(LogLevel loglvl)
{
    std::string str;

    switch (loglvl)
    {
        case ERROR:
            return (str = "  [ERROR]  ");
        case INFO:
            return (str = "  [INFO]   ");
        case DEBUG:
            return (str = "  [DEBUG]  ");
        default:
            return (str);
    }
}

// =======   Static member functions   ======= //
/*
logs an message depending on the logger settings
*/
void    Logger::log(const char *color, LogLevel level, const char *format, ...)
{
    if (_state == OFF)
        return ;
    if (level > _level)
        return ;

    char        output[250];
    va_list     args;

    va_start(args, format);
    vsnprintf(&output[0], 250, format, args);
    va_end(args);

    if (_output == OUT_FILE)
    {
        std::ofstream outFile(LOGFILE_NAME, std::ios::app);

        if (outFile.is_open())
        {
            outFile << getTimestamp() << getLogLevelStr(level) << output << std::endl;
            outFile.close();
        }
        else
            std::cerr << RED << getTimestamp() << getLogLevelStr(ERROR) << "Could not open file: " << LOGFILE_NAME << RESET << std::endl;
    }
    else if (_output == STDOUT)
        std::cout << color << getTimestamp() << getLogLevelStr(level) << output << RESET << std::endl;
}
