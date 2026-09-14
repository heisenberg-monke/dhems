#include "Doctor.hpp"

namespace dhems
{
    void Specialization::load(const nlohmann::json &j)
    {
        auto it = j.find("id");
        if(it != j.end())
            id = it->get<uint64_t>();
        else
            id = nextID<Specialization>();
        name = j.at("name").get<std::string>();
        code = j.at("code").get<uint64_t>();
    }

    void Specialization::save(nlohmann::json &j) const 
    {
        j = nlohmann::json{
            {"id"}, id,
            {"name", name},
            {"code", code}
        };
    }

    void Department::load(const nlohmann::json &j)
    {
        auto it = j.find("id");
        if(it != j.end())
            id = it->get<uint64_t>();
        else
            id = nextID<Department>();
    }

    void Department::save(nlohmann::json &j) const
    {
        j = nlohmann::json {
            {"id", id}
        };
    }

    void Doctor::load(const nlohmann::json &j)
    {
        auto it = j.find("id");
        if(it != j.end())
            id = it->get<uint64_t>();
        else
            id = nextID<Doctor>();
        name            = j.at("name").get<std::string>();
        experience      = j.at("experience").get<std::string>();
        specialization  = j.at("specialization").get<std::vector<uint64_t>>();
        phone           = j.at("phone").get<std::vector<std::string>>();
        timeTable       = j.at("timeTable").get<TimeTable<DoctorTimeSlot>>();
        department      = j.at("department").get<uint64_t>();
    }

    void Doctor::save(nlohmann::json &j) const
    {
        j = nlohmann::json {
            {"id",              id},
            {"name",            name},
            {"experience",      experience},
            {"specialization",  specialization},
            {"phone",           phone},
            {"timeTable",       timeTable},
            {"department",      department}
        };
    }
}