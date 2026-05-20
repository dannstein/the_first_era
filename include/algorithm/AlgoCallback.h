#pragma once
#include <functional>
#include <vector>

using AlgoCallback = std::function<void(const std::vector<int>& route, double cost)>;
