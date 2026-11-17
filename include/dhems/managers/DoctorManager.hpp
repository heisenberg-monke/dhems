#pragma once

#include "dhems/data/Doctor.hpp"
#include "dhems/data/Patient.hpp"

#include <cstdint>
#include <vector>

namespace dhems
{
    class DoctorManager
    {
    public:
        void registerDoctor(Doctor* doctor);
        Doctor* search(uint64_t id) const;
        bool erase(uint64_t id);
        Doctor* assign(Patient* patient);
        std::vector<Doctor*> list() const;
        void release(Patient* patient);
    };
}
