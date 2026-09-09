#include "Logger.hpp"

namespace dhems
{
    namespace Logger
    {
        static inline bool g_debug = false;

        void setDebug(bool debug) {
            g_debug = debug;
        }

        bool debugEnabled() {
            return g_debug;
        }

        std::ostream &info()
        {
            std::clog << "[INFO] ";
            return std::clog;
        }

        std::ostream &warn()
        {
            std::cout << "[WARN] ";
            return std::cout;
        }

        std::ostream &err()
        {
            std::cerr << "[ERROR] ";
            return std::cerr;
        }

        
    }
}