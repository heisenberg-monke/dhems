#include "logger.hpp"

namespace gen
{
    namespace logger
    {
        static inline bool DEBUG = false;
        static inline bool WARNINGS = true;

        bool is_debug_enabled() {
            return DEBUG;
        }

        bool is_warning_enabled() {
            return is_debug_enabled() && WARNINGS;
        }

        void set_debug(bool debug) {
            DEBUG = debug;
        }

        void set_warnings(bool warnings) {
            warnings = WARNINGS;
        }

        void log(std::ostream& out, const char* level, const char* func, const std::string& msg)
        {
            if(level)
                out << '[' << level << "] ";

            if(func)
                out << func << ": ";

            out << msg;
        }
    };
}