#pragma once

#include "Patient.hpp"

#include <queue>

namespace dhems
{
    class PatientManager
    {
        std::queue<size_t> m_regular;
        std::priority_queue<size_t> m_priority;

    public:
        void registerPatient(size_t id);
        void processNextPatient();
    };
}