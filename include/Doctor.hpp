#pragma once

#include "HospitalData.hpp"
#include "TimeTable.hpp"

namespace dhems
{
    class Specialization : public HospitalData
    {
    public:
        std::string name;
        uint64_t code;

        void load(const nlohmann::json &j) override;
        void save(nlohmann::json &j) const override;
    };

    class Department : public HospitalData
    {
    public:
        void load(const nlohmann::json &j) override;
        void save(nlohmann::json &j) const override;
    };

    class TimeTable;
    
    class Doctor : public HospitalData
    {
    public:
        std::string name;
        std::string experience;
        std::vector<uint64_t> specialization;
        std::vector<std::string> phone;
        TimeTable table;
        uint64_t department;

        void load(const nlohmann::json &j) override;
        void save(nlohmann::json &j) const override;
    };
}