#ifndef LOGGER_H
#define LOGGER_H
#pragma once

#include <iostream>
#include <fstream>
#include <mutex>
#include <string>
#include <ctime>
#include <cstdarg>
#include <cstdio>

enum LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3,
    FATAL = 4
};

class Logger {
public:
    static Logger& getInstance();

    void log(LogLevel level, const char* format, ...);
    void setLogLevel(LogLevel level);

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::ofstream logFile;
    std::mutex logMutex;
    LogLevel currentLevel;

    std::string getTimeStamp();
    std::string logLevelToString(LogLevel level);
};

#define LOG_DEBUG(...) Logger::getInstance().log(DEBUG, __VA_ARGS__)
#define LOG_INFO(...) Logger::getInstance().log(INFO, __VA_ARGS__)
#define LOG_WARN(...) Logger::getInstance().log(WARN, __VA_ARGS__)
#define LOG_ERROR(...) Logger::getInstance().log(ERROR, __VA_ARGS__)
#define LOG_FATAL(...) Logger::getInstance().log(FATAL, __VA_ARGS__)

#endif // LOGGER_H