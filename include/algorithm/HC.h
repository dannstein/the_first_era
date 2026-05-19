#pragma once
#include <vector>
#include "core/TSP.h"

class HC {
public:
    std::pair<std::vector<int>, double> Hill(const std::vector<int>& solution, double value, const TSP& tsp);

    std::pair<std::vector<int>, double> Neighbors(const std::vector<int>& solution, const TSP& tsp);
};
