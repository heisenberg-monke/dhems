#include "ResourceManager.hpp"
#include "FileSystem.hpp"

namespace dhems
{
    template <Data T>
    void ResourceManager<T>::updateResource(uint64_t id, const std::string &body)
    {
        auto &patient = FileSystem::fs().find<T>(id);
        patient = nlohmann::json::parse(body).get<T>();
    }

    template <Data T>
    void ResourceManager<T>::exportResource(uint64_t id, std::string &buffer) const
    {
        nlohmann::json j = FileSystem::fs().find<T>(id);
        buffer = j.dump();
    }
}