#include "Logger.h"

Logger& Logger::instance()
{
    static Logger logger;
    return logger;
}

void Logger::setLogLevel(int level)
{
    m_log_level= level;
}

void Logger::log(std::string msg)
{
    switch(m_log_level)
    {
        case INFO:{
            break;
        }
        case ERROR:{
            break;
        }
        case FATAL:{
            break;
        }
        case DEBUG:{
            break;
        }
        default:{
            break;
        }
    }
    
}