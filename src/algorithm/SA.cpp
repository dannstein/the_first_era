#include "algorithm/SA.h"
#include "core/Randomize.h"
#include <random>
#include <cmath>

std::pair<std::vector<int>, double> SA::Neighbor(const std::vector<int>& solution, const TSP& tsp,
                                                   int fixedStart) {
    int size = solution.size();
    int i, j;
    if (fixedStart >= 0) {
        std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<int> distr(1, size - 1);
        i = distr(gen);
        j = distr(gen);
        while (j == i) j = distr(gen);
    } else {
        i = Randomize::randomNumber(size);
        j = Randomize::randomNumber(size);
        while (j == i) j = Randomize::randomNumber(size);
    }

    std::vector<int> s = solution;
    std::swap(s[i], s[j]);

    return {s, tsp.evaluate(s)};
}

std::pair<std::vector<int>, double> SA::Annealing(const std::vector<int>& solution, double value,
                                                    const TSP& tsp, int fixedStart, AlgoCallback cb,
                                                    SAParams params) {
    std::vector<int> current = solution;
    double current_value = value;

    std::vector<int> best_solution = current;
    double best_value = current_value;

    double temperature = params.TI;

    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<double> prob(0.0, 1.0);

    int n = (int)solution.size();

    while (temperature > params.TF) {
        for (int k = 0; k < n; k++) {
            auto [new_solution, new_value] = Neighbor(current, tsp, fixedStart);

            if (new_value < current_value) {
                current = new_solution;
                current_value = new_value;

                if (current_value < best_value) {
                    best_solution = current;
                    best_value = current_value;
                }
            } else {
                double d = new_value - current_value;
                if (prob(gen) < std::exp(-d / temperature)) {
                    current = new_solution;
                    current_value = new_value;
                }
            }

            if (cb) cb(current, current_value);
        }
        temperature *= params.FR;
    }

    return {best_solution, best_value};
}
