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

    class DoctorTimeSlot : public HospitalData
    {
    public:
        bool available = false;

        inline void load(const nlohmann::json &j) override {
            available = j.at("available").get<bool>();
        }

        inline void save(nlohmann::json &j) const override {
            j = {
                {"available", available}
            };
        }
    };

    class Doctor : public HospitalData
    {
    public:
        std::string name;
        std::string experience;
        std::vector<uint64_t> specialization;
        std::vector<std::string> phone;
        TimeTable<DoctorTimeSlot> timeTable;
        uint64_t department;

        void load(const nlohmann::json &j) override;
        void save(nlohmann::json &j) const override;
    };
}