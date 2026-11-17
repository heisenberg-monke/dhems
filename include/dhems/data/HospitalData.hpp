#pragma once

#include <cstdint>

#include <type_traits>

#include <nlohmann/json.hpp>

namespace dhems 
{
    struct HospitalData 
    {
        uint64_t id;

        virtual ~HospitalData() = default;

        virtual void toJSON(nlohmann::json& j) const = 0;
        virtual void fromJSON(const nlohmann::json& j) = 0;
    };

    template <typename T>
    concept HD = std::is_base_of_v<HospitalData, T>;

    template <HD T>
    inline void to_json(nlohmann::json& j, const T& hd) {
        hd.toJSON(j);
    }

    template <HD T>
    inline void from_json(const nlohmann::json& j, T& hd) {
        hd.fromJSON(j);
    }
}
