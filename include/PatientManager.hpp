#pragma once

#include <queue>
#include <string>

#include "Patient.hpp"

namespace dhems
{
    class PatientManager
    {
        std::queue<Patient *> m_regular;
        std::priority_queue<Patient *, std::vector<Patient *>, PatientComparator> m_priority;

        Patient *findPatient(uint64_t id);
        Patient *createPatient(const std::string &body) const;

    public:
        void registerPatient(const std::string &body);
        void updatePatient(uint64_t id, const std::string &body);
        void exportPatient(uint64_t id, std::string &buffer) const;

        Patient *nextPatient(VisitType type);
    };
}