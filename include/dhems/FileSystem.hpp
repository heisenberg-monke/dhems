#pragma once

#include "data/HospitalData.hpp"
#include "utils/Utils.hpp"

#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace dhems
{
    class FileSystem
    {
        using Entries = std::unordered_map<uint64_t, std::unique_ptr<HospitalData>>;

        std::unordered_map<std::type_index, Entries> data;

        FileSystem() = default;

    public:
        FileSystem(const FileSystem&) = delete;
        FileSystem(FileSystem&&) = delete;
        FileSystem& operator=(const FileSystem&) = delete;
        FileSystem& operator=(FileSystem&&) = delete;

        static FileSystem& getInstance()
        {
            static FileSystem instance;
            return instance;
        }

        template <HD T>
        T* get(uint64_t id) const
        {
            const auto group = data.find(typeid(T));

            if (group == data.end()) 
                return nullptr;
            const auto entry = group->second.find(id);

            return entry == group->second.end() ? nullptr : static_cast<T*>(entry->second.get());
        }

        template <HD T>
        std::vector<T*> getAll() const
        {
            std::vector<T*> result;
            const auto group = data.find(typeid(T));
            if (group == data.end()) return result;
            result.reserve(group->second.size());
            for (const auto& [id, entry] : group->second)
                result.push_back(static_cast<T*>(entry.get()));
            return result;
        }

        template <HD T>
        bool erase(uint64_t id)
        {
            const auto group = data.find(typeid(T));
            
            if (group == data.end()) 
                return false;

            return group->second.erase(id) != 0;
        }

        template <HD T>
        T* getOrCreate(uint64_t id)
        {
            auto& entries = data[typeid(T)];
            const auto found = entries.find(id);

            if (found != entries.end()) 
                return static_cast<T*>(found->second.get());

            auto entry = std::make_unique<T>();
            T* result = entry.get();

            entry->id = id;
            entries.emplace(id, std::move(entry));

            return result;
        }

        template <HD T>
        void saveAll(const std::filesystem::path& filePath) const
        {
            nlohmann::json entries = nlohmann::json::array();
            const auto group = data.find(typeid(T));

            if (group != data.end())
            {
                for (const auto& [id, entry] : group->second) 
                    entries.push_back(*static_cast<const T*>(entry.get()));
            }
                
            if (filePath.has_parent_path()) 
                std::filesystem::create_directories(filePath.parent_path());

            std::ofstream file(filePath);

            if (!file) 
                throw std::runtime_error("Could not open JSON file for writing: " + filePath.string());

            file << entries.dump(2) << '\n';

            if (!file) 
                throw std::runtime_error("Could not write JSON file: " + filePath.string());
            LOG_INFO(std::format("Saved {} records to {}", entries.size(), filePath.string()));
        }

        template <HD T>
        void loadAll(const std::filesystem::path& filePath)
        {
            std::ifstream file(filePath);

            if (!file) 
                throw std::runtime_error("Could not open JSON file for reading: " + filePath.string());

            const auto entries = nlohmann::json::parse(file);

            if (!entries.is_array()) 
                throw std::runtime_error("Expected a JSON array in: " + filePath.string());

            auto& group = data[typeid(T)];

            for (const auto& value : entries)
            {
                auto entry = std::make_unique<T>();
                
                value.get_to(*entry);
                group[entry->id] = std::move(entry);
            }
            LOG_INFO(std::format("Loaded {} records from {}", entries.size(), filePath.string()));
        }
    };
}
