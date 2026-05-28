#pragma once
#include <vector>
#include <random>
#include "core/TSP.h"
#include "algorithm/AlgoCallback.h"
#include "algorithm/AlgoParams.h"

class GA {
public:
    static constexpr int    POP_SIZE         = 200;
    static constexpr int    GENERATIONS      = 500;
    static constexpr double MUTATION_RATE    = 0.08; // raised: more diversity, less premature convergence
    static constexpr int    TOURNAMENT_SIZE  = 3;    // kept low: better diversity for this problem size
    static constexpr double GI               = 0.1;  // Generation Interval: % kept as elite
    static constexpr double BR               = 0.8;  // Breeding Rate: % produced by crossover
    static constexpr int    STAGNATION_LIMIT = 75;   // tighter: stop sooner when truly stuck
    // Remainder (1 - GI - BR) = 10% random injections for diversity

    std::pair<std::vector<int>, double> run(const TSP& tsp, int size,
                                             int fixedStart = -1,
                                             AlgoCallback cb = nullptr,
                                             GAParams params = {});

private:
    struct Individual {
        std::vector<int> route;
        double fitness;
    };

    using Population = std::vector<Individual>;

    Population       initPopulation(const TSP& tsp, int size, std::mt19937& gen,
                                     int fixedStart, const GAParams& params);
    Individual       selection(const Population& pop, std::mt19937& gen,
                                int tournamentSize);
    std::vector<int> crossover(const std::vector<int>& p1, const std::vector<int>& p2,
                                std::mt19937& gen, int fixedStart = -1);
    void             mutate(std::vector<int>& solution, std::mt19937& gen,
                             int fixedStart, double mutationRate);
};
