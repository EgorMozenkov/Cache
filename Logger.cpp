#include <cstdarg>
#include <cstdio>

#include "Logger.hpp"

namespace Log 
{
    LogLevel log_level = TRACE;


    void trace(LogLevel level, const char *format, ...) {

        if (level <= log_level) {

            va_list ap;
            va_start(ap, format);
            vprintf(format, ap);
            va_end(ap);
        }
    }

    LogLevel set_log_level(LogLevel level) {
    LogLevel old_log_level = log_level;
    log_level = level;
    return old_log_level;
    }
}
