#pragma once

#include <iostream>

#include <string>

namespace Utils
{
    enum class LogLevel
    {
        OUT,
        INFO,
        WARN,
        ERROR,
    };

    struct Logger
    {
        static void Log(LogLevel level, const std::string& message)
        {
            std::string lStr = "";
            std::ostream* out = &std::cout;

            switch(level)
            {
                case LogLevel::INFO:
                    lStr = "INFO:  ";
                    out = &std::clog;
                    break;

                case LogLevel::WARN:
                    lStr = "WARN:  ";
                    out = &std::cerr;
                    break;

                case LogLevel::ERROR:
                    lStr = "ERROR: ";
                    out = &std::cerr;
                    break;

                default:
                    break;
            }

            *out << lStr << message << '\n';
        }
    };

    #ifdef UTILS_DEBUG
        #define LOG_INFO(msg) Utils::Logger::Log(Utils::LogLevel::INFO, msg)
    #else 
        #define LOG_INFO(msg)
    #endif

    #ifdef UTILS_WARNINGS
        #define LOG_WARN(msg) Utils::Logger::Log(Utils::LogLevel::WARN, msg)
    #else 
        #define LOG_WARN(msg)
    #endif

    #define LOG_ERROR(msg) Utils::Logger::Log(Utils::LogLevel::ERROR, msg)
    #define LOG_OUT(msg) Utils::Logger::Log(Utils::LogLevel::OUT, msg)
}