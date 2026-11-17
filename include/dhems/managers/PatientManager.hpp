#pragma once

#include "dhems/data/Patient.hpp"

#include <cstdint>
#include <queue>
#include <stdexcept>
#include <vector>

namespace dhems
{
    class PatientManager
    {
        struct Priority
        {
            bool operator()(const Patient* left, const Patient* right) const
            {
                if (left->emergency != right->emergency)
                    return !left->emergency;
                    
                return left->id > right->id;
            }
        };

        using Queue = std::priority_queue<Patient*, std::vector<Patient*>, Priority>;
        Queue patients;

    public:
        void registerPatient(Patient* patient);
        Patient* search(uint64_t id) const;
        Patient* update();
        void update(uint64_t id);
        bool erase(uint64_t id);
        std::vector<Patient*> list() const;
    };
}
