#include "dhems/dhems.hpp"

#include "utils/Utils.hpp"

#include <chrono>

int main(int argc, char** argv)
{
    try {
        dhems::App app;
        uint64_t seed = static_cast<uint64_t>(
            std::chrono::system_clock::now().time_since_epoch().count());

        for(int i = 1; i < argc; ++i)
        {   
            std::string_view arg = argv[i];

            if(arg == "--seed")
            {
                if(i+1 >= argc || argv[i+1][0] == '-')
                    throw std::runtime_error("No seed provided.");

                seed = std::stoull(argv[++i]);
            }
        }

        app.run("localhost", 6769, seed);
    }

    catch(const std::exception& e) {
        LOG_ERROR(e.what());
    }

    return 0;
}
