#include "dhems/navigation/CityMapGenerator.hpp"

namespace dhems
{
    nlohmann::json CityMapGenerator::generate() 
    {
        const int N = cfg_.rows * cfg_.cols;
        std::vector<MapNode> nodes(N);
        std::vector<std::pair<uint64_t,uint64_t>> candidateEdges;

        for (int r = 0; r < cfg_.rows; ++r)
        {
            for (int c = 0; c < cfg_.cols; ++c)
            {
                const uint64_t id = static_cast<uint64_t>(r * cfg_.cols + c + 1);
                double x = c * cfg_.cellSize + uniform(-cfg_.jitter, cfg_.jitter);
                double y = r * cfg_.cellSize + uniform(-cfg_.jitter, cfg_.jitter);
                nodes[id-1] = {id, makeName(r, c), x, y};
            }
        }

        // 2. Candidate edges (4-connected + occasional diagonals)
        auto idx = [&](int r, int c) { return static_cast<uint64_t>(r * cfg_.cols + c + 1); };

        for (int r = 0; r < cfg_.rows; ++r)
        {
            for (int c = 0; c < cfg_.cols; ++c)
            {
                if (c + 1 < cfg_.cols) candidateEdges.emplace_back(idx(r,c), idx(r,c+1)); // horizontal
                if (r + 1 < cfg_.rows) candidateEdges.emplace_back(idx(r,c), idx(r+1,c)); // vertical
                if (r + 1 < cfg_.rows && c + 1 < cfg_.cols && bernoulli(cfg_.diagonalProb))
                    candidateEdges.emplace_back(idx(r,c), idx(r+1,c+1));                 // diagonal
                // Longer links represent wider avenues between neighborhoods.
                if (c + 2 < cfg_.cols && bernoulli(0.16))
                    candidateEdges.emplace_back(idx(r,c), idx(r,c+2));
                if (r + 2 < cfg_.rows && bernoulli(0.16))
                    candidateEdges.emplace_back(idx(r,c), idx(r+2,c));
                if (r + 1 < cfg_.rows && c > 0 && bernoulli(0.05))
                    candidateEdges.emplace_back(idx(r,c), idx(r+1,c-1));
            }
        }

        // 3. Kruskal spanning tree
        std::shuffle(candidateEdges.begin(), candidateEdges.end(), rng_);

        std::vector<int> parent(N+1);
        std::iota(parent.begin(), parent.end(), 0);
        auto find = [&](auto& self, int x) -> int {
            return parent[x] == x ? x : parent[x] = self(self, parent[x]);
        };
        auto unite = [&](int a, int b) {
            a = find(find, a); b = find(find, b);
            if (a == b) return false;
            parent[a] = b;
            return true;
        };

        std::vector<std::pair<uint64_t,uint64_t>> roads;
        for (auto [u,v] : candidateEdges)
        {
            if (unite(static_cast<int>(u), static_cast<int>(v)))
                roads.emplace_back(u,v);
        }

        // 4. Add extra edges (cycles) – bias toward center
        const double centerR = (cfg_.rows-1) / 2.0;
        const double centerC = (cfg_.cols-1) / 2.0;

        for (auto [u,v] : candidateEdges)
        {
            if (std::find(roads.begin(), roads.end(), std::pair{u,v}) != roads.end()) continue;

            // distance from center of the two endpoints
            int ur = (u-1) / cfg_.cols, uc = (u-1) % cfg_.cols;
            int vr = (v-1) / cfg_.cols, vc = (v-1) % cfg_.cols;
            double dist = (std::hypot(ur-centerR, uc-centerC) + std::hypot(vr-centerR, vc-centerC)) / 2.0;
            double bias = 1.0 - dist / (std::max(cfg_.rows, cfg_.cols) * 0.7); // higher near center
            bias = std::clamp(bias, 0.15, 1.0);

            if (bernoulli(cfg_.extraEdgeProb * bias))
                roads.emplace_back(u,v);
        }

        // The hospital can be anywhere; retain its node ID in the map data.
        const uint64_t hospitalId = std::uniform_int_distribution<uint64_t>(1, N)(rng_);
        nodes[hospitalId - 1].name = "DHEMS General Hospital";

        // 6. Build JSON
        nlohmann::json j;
        j["hospital"] = hospitalId;
        j["seed"] = cfg_.seed;
        j["nodes"] = nlohmann::json::array();
        for (const auto& n : nodes)
            j["nodes"].push_back({{"id", n.id}, {"name", n.name}, {"x", n.x}, {"y", n.y}});

        j["roads"] = nlohmann::json::array();
        for (auto [a,b] : roads)
            j["roads"].push_back({a, b});

        return j;
    }

    void CityMapGenerator::write(const std::filesystem::path& path)
    {
        std::ofstream out(path);
        if (!out) throw std::runtime_error("Cannot write city map: " + path.string());
        out << generate().dump(2);
    }

    std::string CityMapGenerator::makeName(int r, int c)
    {
        // Simple but city-feeling names; you can expand the tables
        static const char* cols[] = {"Gate","Clinic","Market","Park","Heights","Homes","Square","Museum","Hall","Transit"};
        static const char* specials[] = {"University","Stadium","City Hall","General Hospital","East Riverside","West Market"};

        // a few special names
        if (bernoulli(0.12))
            return specials[static_cast<size_t>(uniform(0, std::size(specials)))];

        static const char* districts[] = {"Northbank", "Maplewood", "Old Town", "Midtown", "Riverside", "Southside", "East End"};
        const size_t district = std::min(static_cast<size_t>(r) * std::size(districts) / cfg_.rows,
                                         std::size(districts) - 1);
        const std::string street = cols[static_cast<size_t>(c) % std::size(cols)];
        return std::string(districts[district]) + " " + street + " " + std::to_string(r + 1) + "-" + std::to_string(c + 1);
    }
}
