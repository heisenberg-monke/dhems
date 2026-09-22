#pragma once

#include "data/doctor.hpp"

namespace dhems
{
    class doctor_manager
    {
        std::unordered_map<size_t, doctor> doctors;

    public:
        void addDoctor(const doctor& d);
        void removeDoctor(size_t doctor_id);

        doctor* findDoctor(size_t doctor_id);

        void get_available_doctors(std::string& result);
        void update_availability(size_t doctor_id, bool available);
    };
}