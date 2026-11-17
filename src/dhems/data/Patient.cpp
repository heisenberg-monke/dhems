#include "dhems/data/Patient.hpp"

namespace dhems
{
    void Patient::toJSON(nlohmann::json& j) const
    {
        j = nlohmann::json{
            {"id", id},
            {"name", name},
            {"department", static_cast<int>(department)},
            {"status", static_cast<int>(status)},
            {"emergency", emergency},
            {"doctorID", doctorID}
        };
    }

    void Patient::fromJSON(const nlohmann::json& j)
    {
        id = j.at("id").get<uint64_t>();
        name = j.at("name").get<std::string>();
        department = static_cast<Department>(j.at("department").get<int>());
        status = static_cast<PatientStatus>(j.at("status").get<int>());
        emergency = j.at("emergency").get<bool>();
        doctorID = j.value("doctorID", uint64_t{0});
    }
}
