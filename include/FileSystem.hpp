#pragma once

#include "HospitalData.hpp"
#include "Logger.hpp"

#include <fstream>

#include <stdexcept>

#include <memory>
#include <format>
#include <typeindex>
#include <filesystem>

#include <unordered_map>

namespace dhems
{
    class FileSystem
    {
        std::unordered_map<std::type_index, DataMap> m_data;

        FileSystem() = default;
        FileSystem(const FileSystem &) = delete;
        FileSystem(FileSystem &&) = delete;

        FileSystem &operator=(const FileSystem &) = delete;
        FileSystem &operator=(FileSystem &&) = delete;

    public:
        static FileSystem &fs()
        {
            static FileSystem instance;
            return instance;
        }

        std::filesystem::path resolveDir(const std::filesystem::path &path) const
        {
            if(path.is_absolute())
                return path;

            return PROJECT_ROOT / path;
        }

        template <Data T>
        inline void insert(std::unique_ptr<T> x)
        {
            if(!x)
                throw std::runtime_error("FileSystem::insert: Corrupted value");

            m_data[typeid(T)][x->id] = std::move(x);
        }

        template <Data T>
        inline void erase(size_t id)
        {
            auto typeIt = m_data.find(typeid(T));

            if(typeIt == m_data.end())
                throw std::runtime_error("FileSystem::erase: No such struct in the filesystem.");

            if(!typeIt->second.erase(id))
                throw std::runtime_error(std::format("FileSystem::erase: No such value in the filesystem: {}", id));
        }

        template <Data T>
        inline T &find(size_t id)
        {
            auto typeIt = m_data.find(typeid(T));

            if(typeIt == m_data.end())
                throw std::runtime_error("FileSystem::find: No such struct in the filesystem.");

            auto dataIt = typeIt->second.find(id);

            if(dataIt == typeIt->second.end())
                throw std::runtime_error(std::format("FileSystem::find: No such value in the filesystem: {}", id));

            return static_cast<T &>(*dataIt->second);
        }

        template <Data T>
        inline const T &find(size_t id) const
        {
            auto typeIt = m_data.find(typeid(T));

            if(typeIt == m_data.end())
                throw std::runtime_error("FileSystem::find: No such struct in the filesystem.");

            auto dataIt = typeIt->second.find(id);

            if(dataIt == typeIt->second.end())
                throw std::runtime_error(std::format("FileSystem::find: No such value in the filesystem: {}", id));

            return static_cast<const T &>(*dataIt->second);
        }

        template <Data T>
        inline DataMap &get()
        {
            auto typeIt = m_data.find(typeid(T));

            if(typeIt == m_data.end())
                throw std::runtime_error("FileSystem::get: No such struct in the filesystem.");

            return typeIt->second;
        }

        template <Data T>
        inline const DataMap &get() const
        {
            auto typeIt = m_data.find(typeid(T));

            if(typeIt == m_data.end())
                throw std::runtime_error("FileSystem::get: No such struct in the filesystem.");

            return typeIt->second;
        }

        template <Data T>
        inline void load(const std::string &body) {
            insert(std::make_unique<T>(nlohmann::json::parse(body).get<T>()));
        }

        template <Data T>
        inline void load(const std::filesystem::path &path)
        {
            std::ifstream file(resolveDir(path));

            if(!file)
                throw std::runtime_error("FileSystem::load: Failed to open file: " + path.string());

            auto j = nlohmann::json::parse(file);

            if(!j.is_array())
                throw std::runtime_error("FileSystem::load: Expected JSON array: " + path.string());

            auto &data = m_data[typeid(T)];
            uint64_t maxID = 0;

            for(const auto &entry : j)
            {
                auto obj = std::make_unique<T>(entry.get<T>());
                const uint64_t id = obj->id;

                if(!data.try_emplace(id, std::move(obj)).second)
                    throw std::runtime_error(std::format("FileSystem::load: Duplicate ID: {}", id));

                maxID = std::max(maxID, id);
            }

            HospitalData::setNextID<T>(maxID);

            LOG_INFO << "Loaded " << maxID << " entries from " << path.string() << '\n';
        }

        template <Data T>
        inline void save(const std::filesystem::path &path) const
        {
            nlohmann::json j = nlohmann::json::array();
            auto typeIt = m_data.find(typeid(T));

            if(typeIt == m_data.end())
                throw std::runtime_error("FileSystem::save: No such struct in the filesystem.");

            for(const auto &[id, obj] : typeIt->second)
                j.push_back(static_cast<const T &>(*obj));

            std::ofstream file(resolveDir(path));

            if(!file)
                throw std::runtime_error("FileSystem::save: Failed to open file: " + path.string());

            file << j.dump(4);

            LOG_INFO << "Generated " << path << '\n';
        }
    };
}