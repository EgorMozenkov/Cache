#pragma once

namespace Log
{
    enum LogLevel { ERROR, INFO, TRACE, DEBUG };

    extern LogLevel log_level;

    void trace(LogLevel level, const char *format, ...);
    LogLevel set_log_level(LogLevel level);
}