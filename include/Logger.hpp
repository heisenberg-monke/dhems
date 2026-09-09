#pragma once

#include <iostream>

namespace dhems
{
    namespace Logger
    {
        std::ostream &info();
        std::ostream &warn();
        std::ostream &err();

        inline std::ostream &display() {
            return std::cout;
        }

        bool debugEnabled();
        void setDebug(bool debug);

        #define LOG_INFO if(dhems::Logger::debugEnabled()) dhems::Logger::info()
        #define LOG_WARN if(dhems::Logger::debugEnabled()) dhems::Logger::warn()
    }
}