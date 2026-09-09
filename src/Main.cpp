#include "dhems.hpp"

int main(int argc, char **argv)
{
    bool debug = false;
    std::string address = "localhost";

    for(int i = 1; i < argc; ++i)
    {
        std::string_view arg = argv[i];

        if(arg == "-d")
            debug = true;

        else if(arg == "-a")
        {
            if(i+1 >= argc || argv[i+1][0] == '-')
                throw std::runtime_error("Usage: -a <address>");

            address = argv[++i];
        }
    }

    dhems::Logger::setDebug(debug);

    try
    {
        dhems::App app;

        app.serve("localhost");
    }

    catch(const std::exception &e)
    {
        dhems::Logger::err()
            << "Test failed: "
            << e.what()
            << '\n';

        return 1;
    }

    return 0;
}