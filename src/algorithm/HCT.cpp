#include "algorithm/HCT.h"
#include "core/Randomize.h"

std::pair<std::vector<int>, double> HCT::Hill(const std::vector<int>& solution, double value,
                                               const TSP& tsp, int t_max, int nodeCount,
                                               int fixedStart, AlgoCallback cb) {
    // Run HC to local optimum from a given starting point.
    auto localSearch = [&](std::vector<int> cur, double curVal)
        -> std::pair<std::vector<int>, double> {
        while (true) {
            auto [nbr, nbrVal] = Neighbors(cur, tsp, fixedStart);
            if (nbrVal < curVal) {
                cur    = nbr;
                curVal = nbrVal;
                if (cb) cb(cur, curVal);
            } else {
                return {cur, curVal};
            }
        }
    };

    auto [best, bestVal] = localSearch(solution, value);

    Randomize rng;
    for (int t = 0; t < t_max; t++) {
        auto restart      = rng.randomSolution(nodeCount, fixedStart);
        double restartVal = tsp.evaluate(restart);
        auto [candidate, candidateVal] = localSearch(restart, restartVal);
        if (candidateVal < bestVal) {
            best    = candidate;
            bestVal = candidateVal;
        }
    }

    return {best, bestVal};
}

std::pair<std::vector<int>, double> HCT::Neighbors(const std::vector<int>& solution, const TSP& tsp,
                                                     int fixedStart) {
    int size     = solution.size();
    int startIdx = (fixedStart >= 0) ? 1 : 0;

    std::vector<int> best_solution;
    double           best_value = 0.0;
    bool             flag       = true;

    for (int i = startIdx; i < size - 1; i++) {
        for (int j = i + 1; j < size; j++) {
            std::vector<int> s = solution;
            std::swap(s[i], s[j]);

            double vs = tsp.evaluate(s);

            if (flag) {
                best_solution = s;
                best_value    = vs;
                flag          = false;
            } else if (vs < best_value) {
                best_solution = s;
                best_value    = vs;
            }
        }
    }

    return {best_solution, best_value};
}
