#include "algorithm/GA.h"
#include "core/Randomize.h"
#include <algorithm>
#include <numeric>

GA::Population GA::initPopulation(const TSP& tsp, int size, std::mt19937& gen) {
    Population pop;
    pop.reserve(POP_SIZE);
    for (int i = 0; i < POP_SIZE; i++) {
        std::vector<int> route = Randomize().randomSolution(size);
        pop.push_back({route, tsp.evaluate(route)});
    }
    return pop;
}

GA::Individual GA::selection(const Population& pop, std::mt19937& gen) {
    std::uniform_int_distribution<int> distr(0, (int)pop.size() - 1);
    int best = distr(gen);
    for (int i = 1; i < TOURNAMENT_SIZE; i++) {
        int candidate = distr(gen);
        if (pop[candidate].fitness < pop[best].fitness)
            best = candidate;
    }
    return pop[best];
}

// Order Crossover (OX): copies a segment from p1, fills the rest from p2 in order
std::vector<int> GA::crossover(const std::vector<int>& p1, const std::vector<int>& p2, std::mt19937& gen) {
    int size = p1.size();
    std::uniform_int_distribution<int> distr(0, size - 1);

    int start = distr(gen);
    int end   = distr(gen);
    if (start > end) std::swap(start, end);

    std::vector<int> child(size, -1);
    std::vector<bool> used(size, false);

    for (int i = start; i <= end; i++) {
        child[i] = p1[i];
        used[p1[i]] = true;
    }

    int pos = (end + 1) % size;
    for (int i = 0; i < size; i++) {
        int gene = p2[(end + 1 + i) % size];
        if (!used[gene]) {
            child[pos] = gene;
            used[gene] = true;
            pos = (pos + 1) % size;
        }
    }

    return child;
}

void GA::mutate(std::vector<int>& solution, std::mt19937& gen) {
    std::uniform_real_distribution<double> prob(0.0, 1.0);
    if (prob(gen) < MUTATION_RATE) {
        std::uniform_int_distribution<int> distr(0, (int)solution.size() - 1);
        int i = distr(gen);
        int j = distr(gen);
        if (i > j) std::swap(i, j);
        std::reverse(solution.begin() + i, solution.begin() + j + 1);
    }
}

std::pair<std::vector<int>, double> GA::run(const TSP& tsp, int size) {
    std::mt19937 gen(std::random_device{}());

    Population pop = initPopulation(tsp, size, gen);

    auto best_it = std::min_element(pop.begin(), pop.end(),
        [](const Individual& a, const Individual& b) { return a.fitness < b.fitness; });
    std::vector<int> best_solution = best_it->route;
    double best_value = best_it->fitness;

    int elite_count  = (int)(GI * POP_SIZE);
    int breed_count  = (int)(BR * POP_SIZE);
    int random_count = POP_SIZE - elite_count - breed_count;
    int stagnation   = 0;

    for (int g = 0; g < GENERATIONS; g++) {
        // Sort population by fitness so elite selection is just taking the front
        std::sort(pop.begin(), pop.end(),
            [](const Individual& a, const Individual& b) { return a.fitness < b.fitness; });

        Population new_pop;
        new_pop.reserve(POP_SIZE);

        // 1. Elite (GI%): carry best individuals unchanged
        for (int i = 0; i < elite_count; i++)
            new_pop.push_back(pop[i]);

        // 2. Offspring (BR%): selection + crossover + mutation, fitness evaluated once at creation
        for (int i = 0; i < breed_count; i++) {
            Individual p1    = selection(pop, gen);
            Individual p2    = selection(pop, gen);
            std::vector<int> route = crossover(p1.route, p2.route, gen);
            mutate(route, gen);
            new_pop.push_back({route, tsp.evaluate(route)});
        }

        // 3. Random (remainder%): fresh solutions to maintain diversity
        for (int i = 0; i < random_count; i++) {
            std::vector<int> route = Randomize().randomSolution(size);
            new_pop.push_back({route, tsp.evaluate(route)});
        }

        pop = std::move(new_pop);

        // Update best and check stagnation
        auto it = std::min_element(pop.begin(), pop.end(),
            [](const Individual& a, const Individual& b) { return a.fitness < b.fitness; });

        if (it->fitness < best_value) {
            best_value    = it->fitness;
            best_solution = it->route;
            stagnation    = 0;
        } else {
            stagnation++;
        }

        if (stagnation >= STAGNATION_LIMIT)
            break;
    }

    return {best_solution, best_value};
}
