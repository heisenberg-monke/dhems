#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

namespace dhems
{
    struct MapNode
    {
        uint64_t id;
        std::string name;
        double x;
        double y;
    };

    struct AmbulanceRoute
    {
        std::vector<MapNode> nodes;
        double distance;
    };

    class CityMap
    {
        struct Edge { uint64_t destination; double distance; };
        std::unordered_map<uint64_t, MapNode> nodes;
        std::unordered_map<uint64_t, std::vector<Edge>> roads;
        uint64_t hospitalId = 0;
        uint64_t mapSeed = 0;

    public:
        void load(const std::filesystem::path& path, uint64_t seed);
        const MapNode& hospital() const;
        const MapNode& getNode(uint64_t id) const;
        nlohmann::json toJSON() const;
        AmbulanceRoute findRoute(uint64_t destinationId) const;
    };
}
