#include "dhems/dhems.hpp"

#include "utils/Utils.hpp"

int main()
{
    try {
        dhems::App app;

        app.run("localhost", 6769);
    }

    catch(const std::exception& e) {
        LOG_ERROR(e.what());
    }

    return 0;
}
