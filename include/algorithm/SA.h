#pragma once
#include <vector>
#include "core/TSP.h"

class SA {
public:
    static constexpr double TI = 1000.0;
    static constexpr double TF = 0.001;
    static constexpr double FR = 0.995;

    std::pair<std::vector<int>, double> Annealing(const std::vector<int>& solution, double value, const TSP& tsp);

private:
    std::pair<std::vector<int>, double> Neighbor(const std::vector<int>& solution, const TSP& tsp);
};
