#pragma once

#include <iostream>

namespace Log
{
    enum LogLevel { ERROR, INFO, TRACE, DEBUG };

    extern LogLevel log_level;
    LogLevel set_log_level(LogLevel level);

    template <typename ... Args>

    void trace(LogLevel level, const Args& ... args) 
    {
        if (level <= log_level) {
            (std::cout << ... << args);
        }
    }
}