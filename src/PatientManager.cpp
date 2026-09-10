#include "PatientManager.hpp"
#include "FileSystem.hpp"
#include <stdexcept>

namespace dhems
{
    Patient *PatientManager::createPatient(const std::string &body) const {
        return FileSystem::fs().insert(std::make_unique<Patient>(nlohmann::json::parse(body).get<Patient>()));
    }

    Patient *PatientManager::registerResource(const std::string &body)
    {
        auto *patient = createPatient(body);

        if(patient->type == VisitType::APPOINTMENT)
            m_regular.emplace(patient);

        else
            m_priority.emplace(patient);

        return patient;
    }

    Patient *PatientManager::nextPatient(VisitType type)
    {
        Patient *next = nullptr;

        if(type == VisitType::ADMISSION)
        {
            if(m_priority.empty())
                throw std::runtime_error("No more patients.");

            next = m_priority.top();
            m_priority.pop();
        }
            
        else
        {
            if(m_regular.empty())
                throw std::runtime_error("No more patients.");
            
            next = m_regular.front();
            m_regular.pop();
        }

        if(!next)
            throw std::runtime_error("No valid patient.");

        return next;
    }
}