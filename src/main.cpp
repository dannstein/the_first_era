#include "core/WorldMap.h"
#include "core/TSP.h"
#include "core/Randomize.h"
#include "core/Node.h"

#include "algorithm/HC.h"
#include "algorithm/HCT.h"
#include "algorithm/SA.h"
#include "algorithm/GA.h"

#include <iostream>
#include <vector>
#include <string>

void printSolution(const std::vector<int>& solution, double value, const Graph& g) {
    std::cout << "\nRoute:\n";
    for (int i = 0; i < (int)solution.size(); i++) {
        const Node& n = g.getNode(solution[i]);
        std::cout << "  [" << (n.id + 1) << "] " << n.name;
        if (i < (int)solution.size() - 1) std::cout << " ->\n";
    }
    const Node& first = g.getNode(solution[0]);
    std::cout << " ->\n  [" << (first.id + 1) << "] " << first.name;
    std::cout << "\n\nTotal cost: " << value << "\n";
}

int main() {
    Graph g = buildWorldMap();
    TSP tsp(36);
    tsp.addGraph(g);

    Randomize randomize;

    std::cout << "=== The First Era: TSP Solver ===\n";

    int choice = -1;
    while (choice != 0) {
        std::cout << "\nSelect algorithm:\n";
        std::cout << "  1. Hill Climbing\n";
        std::cout << "  2. Hill Climbing with Tries\n";
        std::cout << "  3. Simulated Annealing\n";
        std::cout << "  4. Genetic Algorithm\n";
        std::cout << "  0. Exit\n";
        std::cout << "\nChoice: ";
        std::cin >> choice;

        if (choice == 0) break;

        std::vector<int> initial = randomize.randomSolution(36);
        double initial_value = tsp.evaluate(initial);
        std::cout << "\nInitial solution:\n  ";
        for (int id : initial) std::cout << "[" << (id + 1) << "] ";
        std::cout << "\nCost: " << initial_value << "\n";

        std::vector<int> best_solution;
        double best_value = 0.0;

        if (choice == 1) {
            HC hc;
            auto [solution, value] = hc.Hill(initial, initial_value, tsp);
            best_solution = solution;
            best_value = value;
        }else if(choice == 2){
            HCT hct;
            int tries;

            std::cout << "Choose the maximum number of tries: ";
            std::cin >> tries;

            auto [solution, value] = hct.Hill(initial, initial_value, tsp, tries);

            best_solution = solution;
            best_value = value;

        } else if (choice == 3) {
            SA sa;
            auto [solution, value] = sa.Annealing(initial, initial_value, tsp);
            best_solution = solution;
            best_value = value;

        } else if (choice == 4) {
            GA ga;
            auto [solution, value] = ga.run(tsp, 36);
            best_solution = solution;
            best_value = value;

        } else {
            std::cout << "Unknown option.\n";
            continue;
        }

        printSolution(best_solution, best_value, g);
    }

    return 0;
}
