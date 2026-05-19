#pragma once
#include <vector>
#include <random>
#include "core/TSP.h"

class GA {
public:
    static constexpr int    POP_SIZE        = 200;
    static constexpr int    GENERATIONS     = 1000;
    static constexpr double MUTATION_RATE   = 0.07;
    static constexpr int    TOURNAMENT_SIZE = 5;
    static constexpr int    ELITE_SIZE      = 2;

    std::pair<std::vector<int>, double> run(const TSP& tsp, int size);

private:
    using Population = std::vector<std::vector<int>>;

    Population       initPopulation(int size);
    std::vector<int> selection(const Population& pop, const std::vector<double>& fitness, std::mt19937& gen);
    std::vector<int> crossover(const std::vector<int>& p1, const std::vector<int>& p2, std::mt19937& gen);
    void             mutate(std::vector<int>& solution, std::mt19937& gen);
};
