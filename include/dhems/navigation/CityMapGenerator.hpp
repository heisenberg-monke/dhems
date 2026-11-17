#pragma once

#include "CityMap.hpp"

namespace dhems
{
    class CityMapGenerator
    {
    public:
        struct Config
        {
            int rows           = 7;
            int cols           = 7;
            double cellSize    = 150.0;
            double jitter      = 18.0;
            double extraEdgeProb = 0.28;
            double diagonalProb  = 0.08;
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