#include "dhems/navigation/CityMap.hpp"
#include "dhems/navigation/CityMapGenerator.hpp"

#include "utils/Utils.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <queue>
#include <stdexcept>
#include <unordered_set>

namespace dhems
{
    namespace
    {
        double distance(const MapNode& a, const MapNode& b)
        {
            return std::hypot(a.x - b.x, a.y - b.y);
        }
    }

    void CityMap::load(const std::filesystem::path& path, uint64_t seed)
    {
        std::ifstream file(path);
        if (!file) throw std::runtime_error("Could not open city map: " + path.string());

        auto json = nlohmann::json::parse(file);
        const auto fileSeed = json.at("seed").get<uint64_t>();

        if (fileSeed != seed)
            json = CityMapGenerator({.seed = seed}).generate();

        mapSeed = seed;

        std::unordered_map<uint64_t, MapNode> loadedNodes;
        std::unordered_map<uint64_t, std::vector<Edge>> loadedRoads;

        for (const auto& item : json.at("nodes"))
        {
            MapNode node{item.at("id").get<uint64_t>(), item.at("name").get<std::string>(),
                         item.at("x").get<double>(), item.at("y").get<double>()};
            if (!loadedNodes.emplace(node.id, node).second)
                throw std::runtime_error("Duplicate node ID in city map");
        }

        const auto hospital = json.at("hospital").get<uint64_t>();
        if (!loadedNodes.contains(hospital)) throw std::runtime_error("Hospital node is missing from city map");

        for (const auto& road : json.at("roads"))
        {
            const auto from = road.at(0).get<uint64_t>();
            const auto to = road.at(1).get<uint64_t>();
            if (!loadedNodes.contains(from) || !loadedNodes.contains(to) || from == to)
                throw std::runtime_error("Invalid road in city map");
            const double length = distance(loadedNodes.at(from), loadedNodes.at(to));
            loadedRoads[from].push_back({to, length});
            loadedRoads[to].push_back({from, length});
        }

        nodes = std::move(loadedNodes);
        roads = std::move(loadedRoads);
        hospitalId = hospital;
        LOG_INFO(std::format("Loaded city map {} with {} nodes", path.string(), nodes.size()));
    }

    const MapNode& CityMap::hospital() const
    {
        return getNode(hospitalId);
    }

    const MapNode& CityMap::getNode(uint64_t id) const
    {
        const auto found = nodes.find(id);
        if (found == nodes.end()) throw std::out_of_range("Map location not found");
        return found->second;
    }

    nlohmann::json CityMap::toJSON() const
    {
        nlohmann::json result;
        result["hospital"] = hospitalId;
        result["seed"] = mapSeed;
        result["nodes"] = nlohmann::json::array();
        result["roads"] = nlohmann::json::array();

        std::vector<uint64_t> ids;
        ids.reserve(nodes.size());
        for (const auto& [id, _] : nodes) ids.push_back(id);
        std::sort(ids.begin(), ids.end());
        for (uint64_t id : ids)
        {
            const auto& node = nodes.at(id);
            result["nodes"].push_back({{"id", node.id}, {"name", node.name}, {"x", node.x}, {"y", node.y}});
            const auto edges = roads.find(id);
            if (edges != roads.end())
                for (const auto& edge : edges->second)
                    if (id < edge.destination) result["roads"].push_back({id, edge.destination});
        }
        return result;
    }

    AmbulanceRoute CityMap::findRoute(uint64_t destinationId) const
    {
        const auto& destination = getNode(destinationId);
        using QueueItem = std::pair<double, uint64_t>;
        std::priority_queue<QueueItem, std::vector<QueueItem>, std::greater<>> frontier;
        std::unordered_map<uint64_t, double> cost;
        std::unordered_map<uint64_t, uint64_t> previous;
        std::unordered_set<uint64_t> reached;

        cost[hospitalId] = 0.0;
        frontier.push({distance(hospital(), destination), hospitalId});

        while (!frontier.empty())
        {
            const auto current = frontier.top().second;
            frontier.pop();
            if (!reached.insert(current).second) continue;
            if (current == destinationId) break;

            const auto neighbors = roads.find(current);
            if (neighbors == roads.end()) continue;
            for (const auto& edge : neighbors->second)
            {
                const double nextCost = cost.at(current) + edge.distance;
                if (!cost.contains(edge.destination) || nextCost < cost.at(edge.destination))
                {
                    cost[edge.destination] = nextCost;
                    previous[edge.destination] = current;
                    frontier.push({nextCost + distance(getNode(edge.destination), destination), edge.destination});
                }
            }
        }

        if (!cost.contains(destinationId)) throw std::runtime_error("No road route exists to that location");
        std::vector<MapNode> path;
        for (uint64_t id = destinationId;; id = previous.at(id))
        {
            path.push_back(getNode(id));
            if (id == hospitalId) break;
        }
        std::reverse(path.begin(), path.end());
        return {std::move(path), cost.at(destinationId)};
    }
}
