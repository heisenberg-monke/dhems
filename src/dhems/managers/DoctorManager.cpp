#include "dhems/managers/DoctorManager.hpp"
#include "dhems/FileSystem.hpp"
#include "utils/Utils.hpp"

#include <algorithm>
#include <format>
#include <stdexcept>

namespace dhems
{
    void DoctorManager::registerDoctor(Doctor* doctor)
    {
        if (doctor == nullptr) 
            throw std::invalid_argument("Cannot register a null doctor");

        auto& fs = FileSystem::getInstance();
        Doctor* existing = fs.get<Doctor>(doctor->id);

        if (existing != nullptr && existing != doctor)
            throw std::invalid_argument("A doctor with this ID already exists");

        if (existing == nullptr) 
        {
            Doctor* stored = fs.getOrCreate<Doctor>(doctor->id);

            stored->name = doctor->name;
            stored->department = doctor->department;
            stored->busy = doctor->busy;
            stored->patientID = doctor->patientID;
        }

        LOG_INFO(std::format("Registered doctor id={} name='{}'", doctor->id, doctor->name));
    }

    Doctor* DoctorManager::search(uint64_t id) const {
        return FileSystem::getInstance().get<Doctor>(id);
    }

    bool DoctorManager::erase(uint64_t id)
    {
        Doctor* doctor = search(id);

        if (doctor == nullptr) 
            return false;

        if (doctor->busy || doctor->patientID != 0)
            throw std::logic_error("Cannot erase a doctor who is assigned to a patient");

        const bool removed = FileSystem::getInstance().erase<Doctor>(id);

        LOG_INFO(std::format("Doctor erase id={} result={}", id, removed ? "erased" : "not found"));

        return removed;
    }

    Doctor* DoctorManager::assign(Patient* patient)
    {
        if (patient == nullptr) 
            throw std::invalid_argument("Cannot assign a doctor to a null patient");

        auto doctors = list();

        std::sort(doctors.begin(), doctors.end(), [](const Doctor* left, const Doctor* right) {
            return left->id < right->id;
        });

        for (Doctor* doctor : doctors) 
        {
            if (doctor->department == patient->department && !doctor->busy) 
            {
                doctor->busy = true;
                doctor->patientID = patient->id;
                patient->doctorID = doctor->id;

                LOG_INFO(std::format("Assigned doctor id={} name='{}' to patient id={} name='{}'", doctor->id, doctor->name, patient->id, patient->name));

                return doctor;
            }
        }
        LOG_WARN(std::format("No available doctor for patient id={} department={}", patient->id, static_cast<int>(patient->department)));

        return nullptr;
    }

    std::vector<Doctor*> DoctorManager::list() const {
        return FileSystem::getInstance().getAll<Doctor>();
    }

    void DoctorManager::release(Patient* patient)
    {
        if (patient == nullptr || patient->doctorID == 0) 
            return;

        if (Doctor* doctor = search(patient->doctorID)) 
        {
            doctor->busy = false;
            doctor->patientID = 0;

            LOG_INFO(std::format("Released doctor id={} from patient id={}", doctor->id, patient->id));
        }
        
        patient->doctorID = 0;
    }
}
