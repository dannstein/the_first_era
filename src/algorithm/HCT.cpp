#include "algorithm/HCT.h"

std::pair<std::vector<int>, double> HCT::Hill(const std::vector<int>& solution, double value, const TSP& tsp, int t_max) {
    std::vector<int> current = solution;
    double current_value = value;

    std::vector<int> best_solution = current;
    double best_value = current_value;

    int t = 0;

    while (true) {
        auto [new_solution, new_value] = Neighbors(current, tsp);

        if (new_value < current_value) {
            current = new_solution;
            current_value = new_value;
            t = 0;

            if (current_value < best_value) {
                best_solution = current;
                best_value = current_value;
            }
        } else if (t < t_max) {
            current = new_solution;
            current_value = new_value;
            t++;
        } else {
            return {best_solution, best_value};
        }
    }
}

std::pair<std::vector<int>, double> HCT::Neighbors(const std::vector<int>& solution, const TSP& tsp) {
    int size = solution.size();

    std::vector<int> best_solution;
    double best_value = 0.0;
    bool flag = true;

    for (int i = 0; i < size - 1; i++) {
        for (int j = i + 1; j < size; j++) {
            std::vector<int> s = solution;
            std::swap(s[i], s[j]);

            double vs = tsp.evaluate(s);

            if (flag) {
                best_solution = s;
                best_value = vs;
                flag = false;
            } else if (vs < best_value) {
                best_solution = s;
                best_value = vs;
            }
        }
    }

    return {best_solution, best_value};
}
