#pragma once

#include "HospitalData.hpp"

#include <string>
#include <vector>

namespace dhems
{
    enum class Gender
    {
        MALE,
        FEMALE
    };

    enum class PatientPriority
    {
        LOW,
        MEDIUM,
        HIGH,
        SEVERE
    };

    class Patient :  public HospitalData
    {
    public:
        std::string name;
        std::vector<std::string> phone;
        std::string address;
        std::string bloodGroup;
        std::string condition;
        PatientPriority priority;
        Gender gender;
        int age;

        void load(const nlohmann::json &j) override;
        void save(nlohmann::json &j) const override;
    };
}