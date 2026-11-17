#pragma once

#include "dhems/managers/PatientManager.hpp"
#include "dhems/managers/DoctorManager.hpp"
#include "dhems/FileSystem.hpp"

namespace dhems
{
    class App
    {
        static inline std::filesystem::path ROOT_DIR = PROJECT_ROOT; 
        static inline std::filesystem::path WEB_DIR = ROOT_DIR / "web";
        static inline std::filesystem::path DATA_DIR = ROOT_DIR / "data";
        
        PatientManager patientManager;
        DoctorManager doctorManager;

        void serveError(httplib::Response& response, int status, const std::string& message) const;
        void servePatients(httplib::Server& server, FileSystem& fileSystem, uint64_t& nextId);
        void serveDoctors(httplib::Server& server, uint64_t& nextDoctorId);

    public:
        void run(const std::string& address, int port);
    };
}
