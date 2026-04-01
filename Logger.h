#pragma once
#include <string>
#include "noncopyable.h"

#define LOG_INFO(LogmsgFormat, ...)         \
    Logger& logger {Logger::instance()};    \
    logger.setLogLevel(INFO);               \
    char buffer[1024];                      \
    snprintf(buffer, 1024, LogmsgFormat, ##__VA_ARGS__);\
    logger.log(buf);

#define LOG_ERROR(LogmsgFormat, ...)         \
    Logger& logger {Logger::instance()};    \
    logger.setLogLevel(ERROR               \
    char buffer[1024];                      \
    snprintf(buffer, 1024, LogmsgFormat, ##__VA_ARGS__);\
    logger.log(buf);

#define LOG_FATAL(LogmsgFormat, ...)         \
    Logger& logger {Logger::instance()};    \
    logger.setLogLevel(FATAL);               \
    char buffer[1024];                      \
    snprintf(buffer, 1024, LogmsgFormat, ##__VA_ARGS__);\
    logger.log(buf);    \
    exit(-1);

#ifdef DEBUG_MOD
#define LOG_DEBUG(LogmsgFormat, ...)         \
    Logger& logger {Logger::instance()};    \
    logger.setLogLevel(DEBUG);               \
    char buffer[1024];                      \
    snprintf(buffer, 1024, LogmsgFormat, ##__VA_ARGS__);\
    logger.log(buf);
#endif

enum LogLevel
{
    INFO,   // 普通信息
    ERROR,  // 错误信息
    FATAL,  // 崩溃信息
    DEBUG   // 调试信息
};

class Logger: noncopyable
{
    private:
        int m_log_level;
    public:
        static Logger& instance();
        void setLogLevel(int level);
        void log(std::string msg);
    private:
        Logger() = default;
};