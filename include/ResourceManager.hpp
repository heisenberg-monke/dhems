#pragma once

#include "HospitalData.hpp"
#include "FileSystem.hpp"

namespace dhems
{
    template <Data T>
    class ResourceManager
    {
    public:
        virtual ~ResourceManager() = default;

        virtual T *registerResource(const std::string &body) = 0;

        inline void updateResource(uint64_t id, const std::string &body)
        {
            auto &patient = FileSystem::fs().find<T>(id);
            patient = nlohmann::json::parse(body).get<T>();
        }

        inline void exportResource(uint64_t id, std::string &buffer) const
        {
            nlohmann::json j = FileSystem::fs().find<T>(id);
            buffer = j.dump();
        }
    };
}