#pragma once
#include <string>

#define LOGMESSAGESIZE 1024

enum LogLevel {
    NONE = -1,
    LOG  = 0,
    WARN,
    ERR,
    CRIT,
    INFO,
    TRACE,
};

namespace Debug {
    inline bool quiet = false, verbose = false;
    void        log(LogLevel level, const char* fmt, ...);
};