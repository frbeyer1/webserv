#pragma once

#include "Webserv.hpp"

enum LogLvl
{
    ERROR,
    INFO,
    DEBUG,
};

enum OutputMode
{
    STDOUT,
    OUT_FILE,
};

enum State
{
    ON,
    OFF,
};

class Logger
{
private:
    static State       _state;
    static LogLvl      _loglvl;
    static OutputMode  _mode;

public:

// Setter
    static void setState(State state);
    static void setOutputMode(OutputMode mode);
    static void setLogLvl(LogLvl loglvl);

// Static member functions
    static void log(const char *color, LogLvl loglvl, const char *msg);

};
