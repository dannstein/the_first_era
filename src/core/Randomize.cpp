#include "core/Randomize.h"

std::vector<int> Randomize::randomSolution(int size) {
    std::vector<int> result(size);
    std::iota(result.begin(), result.end(), 0);
    std::shuffle(result.begin(), result.end(), std::mt19937{std::random_device{}()});
    return result;
}

int Randomize::randomNumber(int size) {
    std::random_device rd;

    std::mt19937 gen(rd());

    std::uniform_int_distribution<> distr(0, size-1);

    return distr(gen);
}
