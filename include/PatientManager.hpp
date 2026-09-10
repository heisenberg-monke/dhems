#pragma once

#include <queue>

#include "ResourceManager.hpp"
#include "Patient.hpp"

namespace dhems
{
    class PatientManager : public ResourceManager<Patient>
    {
        std::queue<Patient *> m_regular;
        std::priority_queue<Patient *, std::vector<Patient *>, PatientComparator> m_priority;

        Patient *findPatient(uint64_t id);
        Patient *createPatient(const std::string &body) const;

    public:
        Patient *registerResource(const std::string &body) override;
        Patient *nextPatient(VisitType type);
    };
}