#include "App.hpp"
#include "FileSystem.hpp"
#include "Logger.hpp"
#include "Patient.hpp"

#include "cpp-httplib/httplib.h"
#include <stdexcept>

namespace dhems
{
    static inline constexpr const char *WEB_DIR         = "web";
    static inline constexpr const char *PATIENTS_PATH   = "data/patients.json";
    
    static inline httplib::Server *g_server = nullptr;

    static inline void signalHandler(int)
    {
        if(g_server)
            g_server->stop();
    }

    void App::serve(const std::string &address)
    {
        auto serve400 = [](httplib::Response &res, const std::string &err)
        {
            res.status = 400;
            res.set_content(err, "application/json");
        };

        auto &fs = FileSystem::fs();

        fs.load<Patient>(std::filesystem::path(PATIENTS_PATH));

        httplib::Server server;
        const int port = 6769;

        g_server = &server;

        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);

        server.set_mount_point("/", fs.resolveDir(WEB_DIR));

        server.Post("/api/patients", [&fs, &serve400](const httplib::Request &req, httplib::Response &res)
        {
            try {
                auto id = fs.load<Patient>(req.body);

                nlohmann::json response = 
                {
                    {"success", true},
                    {"id", id}
                };

                res.set_content(response.dump(), "application/json");
            }

            catch(const std::exception &e)
            {
                nlohmann::json response =
                {
                    {"success", false},
                    {"error", e.what()}
                };

                serve400(res, response.dump());
            }
        });

        Logger::display() << "Server running at http://" << address << ":" << port << '\n';

        if(!server.listen(address, port))
            throw std::runtime_error("Failed to start server");

        g_server = nullptr;

        Logger::display() << "Stopped server.\n";
        LOG_INFO << "Saving " << PATIENTS_PATH << '\n';

        fs.save<Patient>(PATIENTS_PATH);
    }
}