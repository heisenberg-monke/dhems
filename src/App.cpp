#include "App.hpp"
#include "FileSystem.hpp"
#include "Logger.hpp"
#include "Patient.hpp"
#include "PatientManager.hpp"

#include "cpp-httplib/httplib.h"

#include <csignal>
#include <filesystem>
#include <stdexcept>

namespace dhems
{
    static constexpr const char *WEB_DIR       = "web";
    static constexpr const char *PATIENTS_PATH = "data/patients.json";

    static httplib::Server *g_server = nullptr;

    static void signalHandler(int)
    {
        if(g_server)
            g_server->stop();
    }

    static void serveJSON(httplib::Response &res, const nlohmann::json &j, int status = 200)
    {
        res.status = status;
        res.set_content(j.dump(), "application/json");
    }

    static void serveJSON(httplib::Response &res, const std::string &str, int status = 200)
    {
        res.status = status;
        res.set_content(str, "application/json");
    }

    static void serve400(httplib::Response &res, const std::string &err) 
    {
        serveJSON(res, 
        {
            {"success", false},
            {"error", err}
        }, 400);
    }

    static void serve404(httplib::Response &res, const std::string &err)
    {
        serveJSON(res, 
        {
            {"success", false},
            {"error", err}
        }, 404);
    }

    void App::serve(const std::string &address)
    {
        auto &fs = FileSystem::fs();

        try {
            fs.load<Patient>(std::filesystem::path(PATIENTS_PATH));
        }

        catch(const std::exception &e) {
            Logger::err() << e.what() << '\n';
        }

        httplib::Server server;
        const int port = 6769;

        g_server = &server;

        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);

        server.set_mount_point("/", fs.resolveDir(WEB_DIR));

        server.Post("/api/patients", [](const httplib::Request &req, httplib::Response &res)
        {
            try {
                PatientManager manager;
                auto *patient = manager.registerResource(req.body);

                serveJSON(res, 
                {
                    {"success", true},
                    {"id", patient->id}
                });
            }

            catch(const std::exception &e) {
                serve400(res, e.what());
            }
        });

        server.Get(R"(/api/patients/(\d+))", [](const httplib::Request &req, httplib::Response &res)
        {
            try {
                const uint64_t id = std::stoull(req.matches[1].str());

                PatientManager manager;
                std::string buffer;

                manager.exportResource(id, buffer);
                serveJSON(res, buffer);
            }

            catch(const std::exception &e) {
                serve404(res, e.what());
            }
        });

        server.Put(R"(/api/patients/(\d+))", [](const httplib::Request &req, httplib::Response &res)
        {
            try
            {
                const uint64_t id = std::stoull(req.matches[1].str());

                PatientManager manager;

                manager.updateResource(id, req.body);

                serveJSON(res,
                {
                    {"success", true},
                    {"id", id}
                });
            }
            catch(const std::exception &e) {
                serve400(res, e.what());
            }
        });

        server.Post("/api/patients/next", [](const httplib::Request &req, httplib::Response &res)
        {
            try
            {
                if(!req.has_param("type"))
                    throw std::runtime_error("Missing patient type.");

                const std::string type = req.get_param_value("type");
                VisitType visitType;

                if(type == "APPOINTMENT")
                    visitType = VisitType::APPOINTMENT;

                else if(type == "ADMISSION")
                    visitType = VisitType::ADMISSION;

                else
                    throw std::runtime_error("Invalid patient type.");

                PatientManager manager;
                Patient *patient = manager.nextPatient(visitType);

                serveJSON(res, *patient);
            }

            catch(const std::exception &e) {
                serve404(res, e.what());
            }
        });

        Logger::display() << "Server running at http://" << address << ":" << port << '\n';

        if(!server.listen(address, port))
            throw std::runtime_error("Failed to start server");

        g_server = nullptr;

        Logger::display() << "\nStopped server.\n";

        LOG_INFO << "Saving " << PATIENTS_PATH << '\n';

        fs.save<Patient>(PATIENTS_PATH);
    }
}