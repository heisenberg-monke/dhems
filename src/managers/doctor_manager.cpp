#include "managers/doctor_manager.hpp"

#include "logger.hpp"

namespace dhems
{
    void doctor_manager::addDoctor(const doctor& d)
    {
        if(!this->doctors.try_emplace(d.doctor_id, d).second)
            throw std::runtime_error(std::format("Doctor ID {} exists already.", d.doctor_id));

        LOG_INFO(std::format("Doctor {} added successfully.\n", d.doctor_id));
    }

    void doctor_manager::removeDoctor(size_t doctor_id)
    {
        if(!this->doctors.erase(doctor_id))
            throw std::runtime_error(std::format("Doctor {} not found.", doctor_id));

        LOG_INFO(std::format("Doctor {} removed successully.\n", doctor_id));
    }

    doctor* doctor_manager::findDoctor(size_t doctor_id)
    {
        auto it = this->doctors.find(doctor_id);

        if(it == this->doctors.end())
            throw std::runtime_error(std::format("Doctor {} does not exist.", doctor_id));

        return &it->second;
    }

    void doctor_manager::get_available_doctors(std::string& result)
    {
        auto available = nlohmann::json::array();

        for(const auto &[id, d] : this->doctors)
        {
            if(d.available)
                available.push_back(d);
        }

        result = available.dump();
    }

    void doctor_manager::update_availability(size_t doctor_id, bool available)
    {
        this->findDoctor(doctor_id)->available = available;
        LOG_INFO(std::format("Doctor {} updated successfully.\n", doctor_id));
    }
}