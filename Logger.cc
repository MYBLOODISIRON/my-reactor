#include <iostream>
#include "Logger.h"
#include "Timestamp.h"

Logger& Logger::instance()
{
    static Logger logger;
    return logger;
}

void Logger::setLogLevel(LogLevel level)
{
    m_log_level= level;
}

void Logger::log(const std::string& msg)
{
    switch(m_log_level)
    {
        case INFO:{
            std::cout << "[INFO]";
            break;
        }
        case ERROR:{
            std::cout << "[ERROR]";
            break;
        }
        case FATAL:{
            std::cout << "[FATAL]";
            break;
        }
        case DEBUG:{
            std::cout << "[DEBUG]";
            break;
        }
        default:{
            break;
        }
    }
    
    std::cout << Timestamp::now().toString() << " : " << msg << std::endl;
}