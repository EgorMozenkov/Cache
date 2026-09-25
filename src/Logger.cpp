#include "Logger.hpp"

namespace Log 
{
    LogLevel log_level = ERROR;

    LogLevel set_log_level(LogLevel level) {
    LogLevel old_log_level = log_level;
    log_level = level;
    return old_log_level;
    }
}