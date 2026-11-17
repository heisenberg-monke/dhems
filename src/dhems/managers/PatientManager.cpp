#include "dhems/managers/PatientManager.hpp"
#include "dhems/FileSystem.hpp"
#include "utils/Utils.hpp"

namespace dhems
{
    void PatientManager::registerPatient(Patient* patient)
    {
        if (patient == nullptr)
            throw std::invalid_argument("Cannot register a null patient");

        Queue pending = patients;
        while (!pending.empty())
        {
            if (pending.top()->id == patient->id)
                throw std::invalid_argument("A patient with this ID is already in the admitted queue");
            pending.pop();
        }

        if (patient->status != PatientStatus::ADMITTED)
            throw std::invalid_argument("Only admitted patients can be registered in the queue");

        if (patient->status == PatientStatus::ADMITTED)
        {
            patients.push(patient);
            LOG_INFO(std::format("Registered patient id={} name='{}'", patient->id, patient->name));
        }
    }

    Patient* PatientManager::search(uint64_t id) const
    {
        Patient* patient = FileSystem::getInstance().get<Patient>(id);
        LOG_INFO(std::format("Patient search id={} result={}", id, patient ? "found" : "not found"));
        return patient;
    }

    Patient* PatientManager::update()
    {
        if (patients.empty())
        {
            LOG_WARN("Appointment queue is empty");
            return nullptr;
        }

        Patient* patient = patients.top();

        if (patient->status != PatientStatus::ADMITTED)
            throw std::logic_error("Only admitted patients can be appointed");

        patients.pop();
        patient->status = PatientStatus::APPOINTED;
        LOG_INFO(std::format("Patient id={} name='{}' appointed", patient->id, patient->name));
        return patient;
    }

    void PatientManager::update(uint64_t id)
    {
        Patient* patient = FileSystem::getInstance().get<Patient>(id);

        if (patient == nullptr)
            throw std::out_of_range("Patient ID not found in filesystem");

        if (patient->status != PatientStatus::APPOINTED)
            throw std::logic_error("Only appointed patients can be discharged");
            
        patient->status = PatientStatus::DISCHARGED;
        LOG_INFO(std::format("Patient id={} name='{}' discharged", patient->id, patient->name));
    }

    bool PatientManager::erase(uint64_t id)
    {
        Queue remaining;

        while (!patients.empty())
        {
            Patient* patient = patients.top();

            patients.pop();

            if (patient->id != id)
                remaining.push(patient);
        }

        patients = std::move(remaining);
        
        const bool erased = FileSystem::getInstance().erase<Patient>(id);
        LOG_INFO(std::format("Patient erase id={} result={}", id, erased ? "erased" : "not found"));
        return erased;
    }

    std::vector<Patient*> PatientManager::list() const
    {
        return FileSystem::getInstance().getAll<Patient>();
    }
}
