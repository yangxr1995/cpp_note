#pragma once

#include "noncopyable.h"
#include <cmath>
#include <cstdio>
#include <string>

#define LOG_INFO(LogMsgFormat, ...)  \
    do {  \
        Logger &logger = Logger::instance();  \
        logger.setLogLevel(INFO); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, LogMsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    } while(0)

#define LOG_ERROR(LogMsgFormat, ...)  \
    do {  \
        Logger &logger = Logger::instance();  \
        logger.setLogLevel(ERROR); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, LogMsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    } while(0)

#define LOG_FATAL(LogMsgFormat, ...)  \
    do {  \
        Logger &logger = Logger::instance();  \
        logger.setLogLevel(FATAL); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, LogMsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    } while(0)


#ifdef MUDEBUG
#define LOG_DEBUG(LogMsgFormat, ...)  \
    do {  \
        Logger &logger = Logger::instance();  \
        logger.setLogLevel(DEBUG); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, LogMsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    } while(0)
#else
#define LOG_DEBUG(LogMsgFormat, ...)
#endif



enum logLevel {
    INFO,
    ERROR,
    FATAL,
    DEBUG,
};

class Logger : public noncopyable {
    public:
        static Logger& instance();
        void setLogLevel(int level);
        void log(std::string msg);

    private:
        int logLevel_;
        Logger() {}
};
