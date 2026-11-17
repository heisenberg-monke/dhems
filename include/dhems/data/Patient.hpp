#pragma once

#include "HospitalData.hpp"
#include "Department.hpp"

namespace dhems 
{
    enum class PatientStatus 
    {
        ADMITTED,
        APPOINTED,
        DISCHARGED
    };

    struct Patient : public HospitalData
    {
        std::string name;
        Department department;
        PatientStatus status = PatientStatus::ADMITTED;
        bool emergency = false;
        uint64_t doctorID = 0;

        void toJSON(nlohmann::json& j) const override;
        void fromJSON(const nlohmann::json& j) override;
    };
}
