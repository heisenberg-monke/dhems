#pragma once

#include <string>

#include <cstdint>

#include "HospitalData.hpp"

namespace dhems
{
    template <Data T>
    class ResourceManager
    {
    public:
        virtual ~ResourceManager() = default;

        virtual T *registerResource(const std::string &body) = 0;
        void updateResource(uint64_t id, const std::string &body);
        void exportResource(uint64_t id, std::string &buffer) const;
    };
}