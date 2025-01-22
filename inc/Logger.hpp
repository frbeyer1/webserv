#pragma once

#include "Webserv.hpp"

enum LogState
{
    ON,
    OFF,
};

enum LogLevel
{
    ERROR,
    INFO,
    DEBUG,
};

enum LogOutput
{
    STDOUT,
    OUTFILE,
};

class Logger
{
private:
    static LogState     _state;
    static LogLevel     _level;
    static LogOutput    _output;

public:
// Setter
    static void setState(LogState state);
    static void setLogLevel(LogLevel level);
    static void setOutputMode(LogOutput output);

// Static member functions
    static void log(const char *color, LogLevel loglvl, const char *format, ...);

};
