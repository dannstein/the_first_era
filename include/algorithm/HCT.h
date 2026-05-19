#pragma once
#include <vector>
#include "core/TSP.h"

class HCT {
public:
    std::pair<std::vector<int>, double> Hill(const std::vector<int>& solution, double value, const TSP& tsp, int t_max);

    std::pair<std::vector<int>, double> Neighbors(const std::vector<int>& solution, const TSP& tsp);
};
