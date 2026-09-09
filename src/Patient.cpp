#include "Patient.hpp"

namespace dhems
{
    NLOHMANN_JSON_SERIALIZE_ENUM(PatientPriority, 
    {
        {PatientPriority::LOW, "LOW"},
        {PatientPriority::MEDIUM, "MEDIUM"},
        {PatientPriority::HIGH, "HIGH"},
        {PatientPriority::SEVERE, "SEVERE"}
    })

    NLOHMANN_JSON_SERIALIZE_ENUM(Gender,
    {
        {Gender::MALE, "MALE"},
        {Gender::FEMALE, "FEMALE"}
    })

    void Patient::load(const nlohmann::json &j)
    {
        auto it = j.find("id");

        if(it != j.end())
            id = it->get<uint64_t>();

        else
            id = nextID<Patient>();

        name          = j.at("name").get<std::string>();
        phone         = j.at("phone").get<std::vector<std::string>>();
        address       = j.at("address").get<std::string>();
        bloodGroup    = j.at("bloodGroup").get<std::string>();
        condition     = j.at("condition").get<std::string>();
        priority      = j.at("priority").get<PatientPriority>();
        gender        = j.at("gender").get<Gender>();
        age           = j.at("age").get<int>();
    }

    void Patient::save(nlohmann::json &j) const
    {
        j = nlohmann::json
        {
            {"id",          id},
            {"name",        name},
            {"phone",       phone},
            {"address",     address},
            {"bloodGroup",  bloodGroup},
            {"condition",   condition},
            {"priority",    priority},
            {"gender",      gender},
            {"age",         age}
        };
    }
}