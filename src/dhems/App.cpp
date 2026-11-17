#include "dhems/App.hpp"

#include "utils/Utils.hpp"
#include "dhems/FileSystem.hpp"

#include <algorithm>
#include <chrono>

namespace dhems
{
    namespace 
    { 
        volatile std::sig_atomic_t stopRequested = 0; 
        
        void handleInterrupt(int) { 
            stopRequested = 1; 
        } 
    }

    void App::serveError(httplib::Response& response, int status, const std::string& message) const
    {
        LOG_ERROR(std::format("HTTP request failed status={} error='{}'", status, message));

        response.status = status;
        response.set_content(nlohmann::json{{"error", message}}.dump(), "application/json");
    }

    void App::servePatients(httplib::Server& server, FileSystem& fileSystem, uint64_t& nextId)
    {
        server.Get("/api/patients", [this](const httplib::Request&, httplib::Response& response) 
        {
            nlohmann::json result = nlohmann::json::array();

            for (Patient* patient : patientManager.list()) 
            {
                nlohmann::json item = *patient;
                Doctor* doctor = doctorManager.search(patient->doctorID);

                item["doctorName"] = doctor ? doctor->name : "";
                result.push_back(std::move(item));
            }

            response.set_content(result.dump(), "application/json");
        });

        server.Post("/api/patients", [this, &fileSystem, &nextId](const httplib::Request& request, httplib::Response& response) 
        {
            try {
                auto body = nlohmann::json::parse(request.body);

                body["id"] = nextId++;
                body["status"] = static_cast<int>(PatientStatus::ADMITTED);
                body["doctorID"] = uint64_t{0};

                Patient* patient = fileSystem.getOrCreate<Patient>(body.at("id").get<uint64_t>());

                body.get_to(*patient);
                patientManager.registerPatient(patient);
                LOG_INFO(std::format("HTTP registered patient id={}", patient->id));
                response.set_content(nlohmann::json{{"id", patient->id}}.dump(), "application/json");
            } 
            
            catch (const std::exception& error) { 
                serveError(response, 400, error.what()); 
            }
        });

        server.Get(R"(/api/patients/(\d+))", [this, &fileSystem](const httplib::Request& request, httplib::Response& response) 
        {
            try {
                const auto id = std::stoull(request.matches[1]);
                Patient* patient = patientManager.search(id);

                if (!patient) 
                { 
                    serveError(response, 404, "Patient not found"); 
                    return; 
                }

                LOG_INFO(std::format("HTTP returned patient id={}", id));

                nlohmann::json json = *patient;
                Doctor* doctor = doctorManager.search(patient->doctorID);

                json["doctorName"] = doctor ? doctor->name : "";
                response.set_content(json.dump(), "application/json");
            } 
            
            catch (const std::exception& error) { 
                serveError(response, 400, error.what()); 
            }
        });

        server.Post("/api/patients/next", [this](const httplib::Request&, httplib::Response& response) 
        {
            try {
                Patient* patient = patientManager.update();

                if (patient == nullptr) 
                {
                    LOG_INFO("Appointment request received; all patients are appointed");
                    response.set_content(nlohmann::json{{"success", false}, {"message", "All patients are appointed"}}.dump(), "application/json");

                    return;
                }

                Doctor* doctor = doctorManager.assign(patient);

                LOG_INFO(std::format("Appointment request completed for patient id={}", patient->id));
                response.set_content(nlohmann::json{{"success", true}, {"name", patient->name}, {"doctorName", doctor ? doctor->name : "No doctor available"}}.dump(), "application/json");
            } 
            
            catch (const std::exception& error) { 
                serveError(response, 400, error.what()); 
            }
        });

        server.Put(R"(/api/patients/(\d+))", [this](const httplib::Request& request, httplib::Response& response) 
        {
            try {
                const auto id = std::stoull(request.matches[1]);
                Patient* patient = patientManager.search(id);

                patientManager.update(id);
                doctorManager.release(patient);

                LOG_INFO(std::format("Discharge request completed for patient id={}", id));
                response.set_content("{\"success\":true}", "application/json");
            } 
            
            catch (const std::exception& error) { 
                serveError(response, 400, error.what()); 
            }
        });

        server.Delete(R"(/api/patients/(\d+))", [this, &fileSystem](const httplib::Request& request, httplib::Response& response) 
        {
            try {
                const auto id = std::stoull(request.matches[1]);

                if (!patientManager.erase(id)) 
                { 
                    serveError(response, 404, "Patient not found"); 
                    return; 
                }

                LOG_INFO(std::format("Erase request completed for patient id={}", id));
                response.set_content("{\"success\":true}", "application/json");
            } 
            
            catch (const std::exception& error) { 
                serveError(response, 400, error.what()); 
            }
        });
    }

    void App::serveDoctors(httplib::Server& server, uint64_t& nextDoctorId)
    {
        server.Get("/api/doctors", [this](const httplib::Request&, httplib::Response& response) 
        {
            nlohmann::json result = nlohmann::json::array();

            for (Doctor* doctor : doctorManager.list()) 
            {
                nlohmann::json item = *doctor;
                Patient* patient = patientManager.search(doctor->patientID);

                item["patientName"] = patient ? patient->name : "";
                result.push_back(std::move(item));
            }

            response.set_content(result.dump(), "application/json");
        });

        server.Post("/api/doctors", [this, &nextDoctorId](const httplib::Request& request, httplib::Response& response) 
        {
            try {
                auto body = nlohmann::json::parse(request.body);

                body["id"] = nextDoctorId++;

                Doctor* doctor = FileSystem::getInstance().getOrCreate<Doctor>(body.at("id").get<uint64_t>());

                body.get_to(*doctor);
                doctorManager.registerDoctor(doctor);
                response.set_content(nlohmann::json{{"id", doctor->id}}.dump(), "application/json");
            } 
            
            catch (const std::exception& error) { 
                serveError(response, 400, error.what()); 
            }
        });

        server.Get(R"(/api/doctors/(\d+))", [this](const httplib::Request& request, httplib::Response& response) 
        {
            try {
                const auto id = std::stoull(request.matches[1]);
                Doctor* doctor = doctorManager.search(id);

                if (!doctor) 
                { 
                    serveError(response, 404, "Doctor not found"); 
                    return; 
                }

                Patient* patient = patientManager.search(doctor->patientID);
                nlohmann::json result = *doctor;

                result["patientName"] = patient ? patient->name : "";
                response.set_content(result.dump(), "application/json");
            } 
            
            catch (const std::exception& error) { 
                serveError(response, 400, error.what()); 
            }
        });

        server.Delete(R"(/api/doctors/(\d+))", [this](const httplib::Request& request, httplib::Response& response) 
        {
            try {
                const auto id = std::stoull(request.matches[1]);
                if (!doctorManager.erase(id))
                { 
                    serveError(response, 404, "Doctor not found"); 
                    return; 
                }

                response.set_content("{\"success\":true}", "application/json");
            } 
            
            catch (const std::exception& error) { 
                serveError(response, 400, error.what()); 
            }
        });

        server.Post("/api/doctors/release", [this](const httplib::Request& request, httplib::Response& response) 
        {
            try {
                const auto body = nlohmann::json::parse(request.body);
                Patient* patient = patientManager.search(body.at("patientId").get<uint64_t>());

                if (!patient) 
                { 
                    serveError(response, 404, "Patient not found"); 
                    return; 
                }

                doctorManager.release(patient);
                response.set_content("{\"success\":true}", "application/json");
            } 
            
            catch (const std::exception& error) { 
                serveError(response, 400, error.what()); 
            }
        });
    }

    void App::serveAmbulances(httplib::Server& server)
    {
        auto cityMap = FileSystem::getInstance().get<CityMap>(1);

        server.Get("/api/city-map", [this, cityMap](const httplib::Request&, httplib::Response& response) {
            response.set_content(nlohmann::json(*cityMap).dump(), "application/json");
        });

        server.Post("/api/ambulance", [this, cityMap](const httplib::Request& request, httplib::Response& response) {
            try {
                const auto body = nlohmann::json::parse(request.body);
                const auto destinationId = body.at("locationId").get<uint64_t>();
                const auto runAlgorithm = [cityMap, destinationId](auto algorithm)
                {
                    const auto start = std::chrono::steady_clock::now();
                    const auto route = (cityMap->*algorithm)(destinationId);
                    const auto elapsed = std::chrono::duration<double, std::milli>(
                        std::chrono::steady_clock::now() - start).count();
                    nlohmann::json routeNodes = nlohmann::json::array();
                    for (const auto& node : route.nodes)
                        routeNodes.push_back({{"id", node.id}, {"name", node.name}, {"x", node.x}, {"y", node.y}});
                    return nlohmann::json{{"route", std::move(routeNodes)},
                                          {"distance", route.distance}, {"timeMs", elapsed}};
                };
                const auto astar = runAlgorithm(&CityMap::findRoute);
                const auto dijkstra = runAlgorithm(&CityMap::findRouteDijkstra);

                response.set_content(nlohmann::json{
                    {"hospital", {{"id", cityMap->hospital().id}, {"name", cityMap->hospital().name}}},
                    {"destination", {{"id", destinationId}, {"name", cityMap->getNode(destinationId).name}}},
                    {"astar", astar}, {"dijkstra", dijkstra}
                }.dump(), "application/json");
            } catch (const std::exception& error) { serveError(response, 400, error.what()); }
        });
    }

    void App::run(const std::string& address, int port, uint64_t seed)
    {
        FileSystem& fileSystem = FileSystem::getInstance();
        const auto patientFile = DATA_DIR / "patients.json";
        const auto doctorFile = DATA_DIR / "doctors.json";
        const auto cityMapFile = DATA_DIR / "city_map.json";

        auto cityMap = fileSystem.getOrCreate<CityMap>(1);
        cityMap->generate(seed);

        if (std::filesystem::exists(patientFile)) 
            fileSystem.loadAll<Patient>(patientFile);

        else LOG_INFO(std::format("No patient data file found at {}; starting empty", patientFile.string()));

        if (std::filesystem::exists(doctorFile))
            fileSystem.loadAll<Doctor>(doctorFile);

        uint64_t nextDoctorId = 1;

        for (Doctor* doctor : fileSystem.getAll<Doctor>())
            nextDoctorId = std::max(nextDoctorId, doctor->id + 1);

        uint64_t nextId = 1;

        for (Patient* patient : fileSystem.getAll<Patient>())
        {
            if (patient->status == PatientStatus::ADMITTED)
                patientManager.registerPatient(patient);
                
            nextId = std::max(nextId, patient->id + 1);
        }

        httplib::Server server;

        servePatients(server, fileSystem, nextId);
        serveDoctors(server, nextDoctorId);
        serveAmbulances(server);

        server.Get("/", [](const httplib::Request&, httplib::Response& response) {
            response.set_file_content((WEB_DIR / "dhems/index.html").string());
        });

        if (!server.set_mount_point("/", WEB_DIR.string()))
            throw std::runtime_error("Failed to mount web directory: " + WEB_DIR.string());

        LOG_OUT(std::format("Listening at http://{}:{}", address, port));

        auto previousHandler = std::signal(SIGINT, handleInterrupt);
        std::atomic_bool watcherDone = false;

        stopRequested = 0;

        std::thread signalWatcher([&server, &watcherDone] 
        {
            while (!watcherDone.load()) 
            {
                if (stopRequested) 
                { 
                    server.stop(); 
                    return; 
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(25));
            }
        });

        const bool listened = server.listen(address, port);

        watcherDone = true;

        signalWatcher.join();
        std::signal(SIGINT, previousHandler);
        fileSystem.saveAll<Patient>(patientFile);
        fileSystem.saveAll<Doctor>(doctorFile);
        fileSystem.saveAll<CityMap>(cityMapFile);

        LOG_INFO("Server stopped; patient data saved");

        if (!listened)
            throw std::runtime_error("Failed to start HTTP server at " + address + ":" + std::to_string(port));
    }
}
