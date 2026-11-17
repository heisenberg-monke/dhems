#pragma once

#include "CityMap.hpp"

namespace dhems
{
    class CityMapGenerator
    {
    public:
        struct Config
        {
            int rows           = 21;
            int cols           = 21;
            double cellSize    = 95.0;
            double jitter      = 24.0;
            double extraEdgeProb = 0.38;
            double diagonalProb  = 0.16;
            uint64_t seed      = 42;
        };

        explicit CityMapGenerator(const Config& cfg)
            : cfg_(cfg), rng_(cfg_.seed) {}

        nlohmann::json generate();

    private:
        Config cfg_;
        std::mt19937_64 rng_;

        void write(const std::filesystem::path& path);

        double uniform(double lo, double hi) {
            return std::uniform_real_distribution<double>(lo, hi)(rng_);
        }
        bool bernoulli(double p) {
            return std::bernoulli_distribution(p)(rng_);
        }

        std::string makeName(int r, int c);
    };
}
