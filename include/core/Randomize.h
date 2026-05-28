#pragma once
#include <vector>
#include <random>
#include <numeric>
#include <algorithm>

class Randomize {
public:
    std::vector<int> randomSolution(int size, int fixedStart = -1);

    static int randomNumber(int size);
};
