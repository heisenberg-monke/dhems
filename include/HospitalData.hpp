#pragma once

#include <concepts>
#include <cstdint>

#include <memory>

#include <unordered_map>

#include <nlohmann/json.hpp>

namespace dhems
{
    class HospitalData
    {
    public:
        uint64_t id;

        virtual ~HospitalData() = default;

        template <typename T>
        static uint64_t &idCount()
        {
            static uint64_t next = 0;
            return next;
        }

        template <typename T>
        static uint64_t nextID() {
            return ++idCount<T>();
        }

        template <typename T>
        static void setNextID(uint64_t next) {
            idCount<T>() = next;
        }

        virtual void load(const nlohmann::json &j) = 0;
        virtual void save(nlohmann::json &j) const = 0;
    };

    using DataMap = std::unordered_map<uint64_t, std::unique_ptr<HospitalData>>;

    template <typename T>
    concept Data = std::derived_from<T, HospitalData>;

    template <Data T>
    void from_json(const nlohmann::json &j, T &obj) {
        obj.load(j);
    }

    template <Data T>
    void to_json(nlohmann::json &j, const T &obj) {
        obj.save(j);
    }
}