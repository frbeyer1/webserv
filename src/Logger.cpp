#include "../inc/Logger.hpp"

State Logger::_state = ON;
LogLvl Logger::_loglvl = ERROR;
OutputMode Logger::_mode = STDOUT;

// ================   Setter   =============== //
void    Logger::setState(State state)
{
    _state = state;
}

void    Logger::setLogLvl(LogLvl loglvl)
{
    _loglvl = loglvl;
}

void    Logger::setOutputMode(OutputMode mode)
{
    _mode = mode;
}

// ================   Utils   ================ //
/*
returns an timestamp with the format [day/month/year  hours:minutes:seconds]
*/
static std::string getTimestamp()
{
    std::time_t t = std::time(NULL);
    std::tm     tm = *std::localtime(&t);
    char        buffer[50];

    std::strftime(buffer, sizeof(buffer), "[%d/%b/%Y  %H:%M:%S]", &tm);
    return (std::string(buffer));
}

/*
returns an string with the loglvl
*/
static std::string getLogLvlStr(LogLvl loglvl)
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
logs an message depending on the log settings
*/
void    Logger::log(const char *color, LogLvl loglvl, std::string msg)
{
    if (_state == OFF)
        return ;
    if (loglvl > _loglvl)
        return ;
    if (_mode == OUT_FILE)
    {
        std::ofstream   outFile(LOGFILE_NAME);

        if (outFile.is_open())
        {
            outFile << getTimestamp() << getLogLvlStr(loglvl) << msg << std::endl;
            outFile.close();
        }
        else
            std::cerr << RED << getTimestamp() << getLogLvlStr(ERROR) << "Could not open file: " << LOGFILE_NAME << RESET << std::endl;
    }
    else if (_mode == STDOUT)
        std::cout << color << getTimestamp() << getLogLvlStr(loglvl) << msg << RESET << std::endl;
}
