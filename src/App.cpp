#include "App.hpp"
#include "FileSystem.hpp"
#include "Logger.hpp"
#include "Patient.hpp"
#include "PatientManager.hpp"

#include "cpp-httplib/httplib.h"

#include <algorithm>
#include <csignal>
#include <filesystem>
#include <stdexcept>
#include <string>

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

    void App::serve(const std::string &address)
    {
        auto serve400 = [](httplib::Response &res, const std::string &err)
        {
            res.status = 400;
            res.set_content(err, "application/json");
        };

        auto serveJson = [](httplib::Response &res,
                            const nlohmann::json &json,
                            int status = 200)
        {
            res.status = status;
            res.set_content(json.dump(), "application/json");
        };

        auto &fs = FileSystem::fs();

        /*
         * Load the existing patients first.
         */
        try {
            fs.load<Patient>(std::filesystem::path(PATIENTS_PATH));
        }

        catch(const std::exception &e) {
            Logger::err() << e.what() << '\n';
        }
        

        /*
         * PatientManager owns the appointment/admission queues.
         */
        PatientManager patientManager;

        httplib::Server server;
        const int port = 6769;

        g_server = &server;

        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);

        /*
         * Serve index.html, app.js and style.css from web/.
         */
        if(!server.set_mount_point("/", fs.resolveDir(WEB_DIR)))
            throw std::runtime_error("Failed to mount web directory");

        /*
         * Register a new patient through PatientManager.
         *
         * POST /api/patients
         */
        server.Post(
            "/api/patients",
            [&patientManager, &fs, &serve400, &serveJson]
            (const httplib::Request &req, httplib::Response &res)
        {
            try
            {
                /*
                 * Validate the JSON before handing it to PatientManager.
                 */
                auto json = nlohmann::json::parse(req.body);

                if(!json.contains("type"))
                    throw std::runtime_error(
                        "Missing required field: type"
                    );

                patientManager.registerPatient(req.body);

                /*
                 * registerPatient() currently returns void, so locate
                 * the newly-created patient by taking the greatest ID.
                 */
                const auto &patients = fs.get<Patient>();

                if(patients.empty())
                    throw std::runtime_error(
                        "Patient was registered but could not be located"
                    );

                auto it = std::max_element(
                    patients.begin(),
                    patients.end(),
                    [](const auto &a, const auto &b)
                    {
                        return a.first < b.first;
                    }
                );

                nlohmann::json response =
                {
                    {"success", true},
                    {"id", it->first}
                };

                serveJson(res, response);
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

        /*
         * Return all currently stored patients.
         *
         * GET /api/patients
         */
        server.Get(
            "/api/patients",
            [&fs, &serveJson, &serve400]
            (const httplib::Request &, httplib::Response &res)
        {
            try
            {
                nlohmann::json patients = nlohmann::json::array();

                for(const auto &[id, object] : fs.get<Patient>())
                {
                    nlohmann::json patient = static_cast<const Patient &>(*object);
                    patient["id"] = id;
                    patients.push_back(std::move(patient));
                }

                serveJson(
                    res,
                    {
                        {"success", true},
                        {"patients", patients}
                    }
                );
            }
            catch(const std::exception &e)
            {
                serve400(
                    res,
                    nlohmann::json{
                        {"success", false},
                        {"error", e.what()}
                    }.dump()
                );
            }
        });

        /*
         * Export one patient.
         *
         * GET /api/patients/<id>
         */
        server.Get(
            R"(/api/patients/(\d+))",
            [&patientManager, &serveJson, &serve400]
            (const httplib::Request &req, httplib::Response &res)
        {
            try
            {
                const uint64_t id =
                    std::stoull(req.matches[1].str());

                std::string buffer;
                patientManager.exportPatient(id, buffer);

                serveJson(
                    res,
                    {
                        {"success", true},
                        {"patient", nlohmann::json::parse(buffer)}
                    }
                );
            }
            catch(const std::exception &e)
            {
                serve400(
                    res,
                    nlohmann::json{
                        {"success", false},
                        {"error", e.what()}
                    }.dump()
                );
            }
        });

        /*
         * Update one patient.
         *
         * PUT /api/patients/<id>
         */
        server.Put(
            R"(/api/patients/(\d+))",
            [&patientManager, &serveJson, &serve400]
            (const httplib::Request &req, httplib::Response &res)
        {
            try
            {
                const uint64_t id =
                    std::stoull(req.matches[1].str());

                /*
                 * Patient::load() generates an ID when one isn't supplied.
                 * Keep the existing ID so the FileSystem key remains valid.
                 */
                auto json = nlohmann::json::parse(req.body);
                json["id"] = id;

                patientManager.updatePatient(id, json.dump());

                serveJson(
                    res,
                    {
                        {"success", true},
                        {"id", id}
                    }
                );
            }
            catch(const std::exception &e)
            {
                serve400(
                    res,
                    nlohmann::json{
                        {"success", false},
                        {"error", e.what()}
                    }.dump()
                );
            }
        });

        /*
         * Remove the next patient from a queue.
         *
         * POST /api/patients/next?type=APPOINTMENT
         * POST /api/patients/next?type=ADMISSION
         */
        server.Post(
            "/api/patients/next",
            [&patientManager, &serveJson, &serve400]
            (const httplib::Request &req, httplib::Response &res)
        {
            try
            {
                if(!req.has_param("type"))
                    throw std::runtime_error(
                        "Missing required query parameter: type"
                    );

                const std::string type = req.get_param_value("type");

                VisitType visitType;

                if(type == "ADMISSION")
                    visitType = VisitType::ADMISSION;
                else if(type == "APPOINTMENT")
                    visitType = VisitType::APPOINTMENT;
                else
                    throw std::runtime_error(
                        "Invalid visit type. Use APPOINTMENT or ADMISSION."
                    );

                Patient *patient =
                    patientManager.nextPatient(visitType);

                nlohmann::json patientJson = *patient;

                serveJson(
                    res,
                    {
                        {"success", true},
                        {"patient", patientJson}
                    }
                );
            }
            catch(const std::exception &e)
            {
                serve400(
                    res,
                    nlohmann::json{
                        {"success", false},
                        {"error", e.what()}
                    }.dump()
                );
            }
        });

        Logger::display()
            << "Server running at http://"
            << address
            << ":"
            << port
            << '\n';

        if(!server.listen(address, port))
            throw std::runtime_error("Failed to start server");

        g_server = nullptr;

        Logger::display() << "Stopped server.\n";
        LOG_INFO << "Saving " << PATIENTS_PATH << '\n';

        fs.save<Patient>(PATIENTS_PATH);
    }
}