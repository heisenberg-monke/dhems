#pragma once

namespace gen
{
    namespace logger
    {
        bool is_debug_enabled();
        bool is_warning_enabled();

        void set_debug(bool debug);
        void set_warnings(bool warnings);

        void log(std::ostream& out, const char* level, const char* func, const std::string& msg);

        #define LOG_INFO(str) if (gen::logger::is_debug_enabled()) gen::logger::log(std::cout, "INFO", __func__, (str))
        #define LOG_WARN(str) if (gen::logger::is_warning_enabled()) gen::logger::log(std::cerr, "WARN", __func__, (str))
        #define LOG_ERROR(str) gen::logger::log(std::cerr, "ERROR", __func__, (str))
        #define LOG_OUT(str) gen::logger::log(std::cout, nullptr, nullptr, (str))
    };
}