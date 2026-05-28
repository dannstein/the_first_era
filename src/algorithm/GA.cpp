#include "algorithm/GA.h"
#include "core/Randomize.h"
#include <algorithm>
#include <numeric>

GA::Population GA::initPopulation(const TSP& tsp, int size, std::mt19937& gen,
                                    int fixedStart, const GAParams& params) {
    Population pop;
    pop.reserve(params.popSize);
    for (int i = 0; i < params.popSize; i++) {
        std::vector<int> route = Randomize().randomSolution(size, fixedStart);
        pop.push_back({route, tsp.evaluate(route)});
    }
    return pop;
}

GA::Individual GA::selection(const Population& pop, std::mt19937& gen, int tournamentSize) {
    std::uniform_int_distribution<int> distr(0, (int)pop.size() - 1);
    int best = distr(gen);
    for (int i = 1; i < tournamentSize; i++) {
        int candidate = distr(gen);
        if (pop[candidate].fitness < pop[best].fitness)
            best = candidate;
    }
    return pop[best];
}

// Order Crossover (OX): copies a segment from p1, fills the rest from p2 in order.
// When fixedStart >= 0, index 0 is preserved; OX operates only on [1, size-1].
std::vector<int> GA::crossover(const std::vector<int>& p1, const std::vector<int>& p2,
                                 std::mt19937& gen, int fixedStart) {
    int size = p1.size();

    if (fixedStart >= 0) {
        int tailSize = size - 1;
        std::uniform_int_distribution<int> distr(0, tailSize - 1);
        int start = distr(gen);
        int end   = distr(gen);
        if (start > end) std::swap(start, end);

        std::vector<int> child(size, -1);
        std::vector<bool> used(size, false);
        child[0] = fixedStart;
        used[fixedStart] = true;

        for (int i = start; i <= end; i++) {
            child[1 + i] = p1[1 + i];
            used[p1[1 + i]] = true;
        }

        int pos = (end + 1) % tailSize;
        for (int i = 0; i < tailSize; i++) {
            int gene = p2[1 + (end + 1 + i) % tailSize];
            if (!used[gene]) {
                child[1 + pos] = gene;
                used[gene] = true;
                pos = (pos + 1) % tailSize;
            }
        }
        return child;
    }

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

void GA::mutate(std::vector<int>& solution, std::mt19937& gen, int fixedStart, double mutationRate) {
    std::uniform_real_distribution<double> prob(0.0, 1.0);
    if (prob(gen) < mutationRate) {
        int low = (fixedStart >= 0) ? 1 : 0;
        std::uniform_int_distribution<int> distr(low, (int)solution.size() - 1);
        int i = distr(gen);
        int j = distr(gen);
        if (i > j) std::swap(i, j);
        std::reverse(solution.begin() + i, solution.begin() + j + 1);
    }
}

std::pair<std::vector<int>, double> GA::run(const TSP& tsp, int size, int fixedStart,
                                              AlgoCallback cb, GAParams params) {
    std::mt19937 gen(std::random_device{}());

    Population pop = initPopulation(tsp, size, gen, fixedStart, params);

    auto best_it = std::min_element(pop.begin(), pop.end(),
        [](const Individual& a, const Individual& b) { return a.fitness < b.fitness; });
    std::vector<int> best_solution = best_it->route;
    double best_value = best_it->fitness;

    int elite_count  = (int)(params.gi * params.popSize);
    int breed_count  = (int)(params.br * params.popSize);
    int random_count = params.popSize - elite_count - breed_count;
    int stagnation   = 0;

    for (int g = 0; g < params.generations; g++) {
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
            Individual p1    = selection(pop, gen, params.tournamentSize);
            Individual p2    = selection(pop, gen, params.tournamentSize);
            std::vector<int> route = crossover(p1.route, p2.route, gen, fixedStart);
            mutate(route, gen, fixedStart, params.mutationRate);
            new_pop.push_back({route, tsp.evaluate(route)});
        }

        // 3. Random (remainder%): fresh solutions to maintain diversity
        for (int i = 0; i < random_count; i++) {
            std::vector<int> route = Randomize().randomSolution(size, fixedStart);
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

        // Pass current generation's best (not all-time best) so the animation shows evolution
        if (cb) cb(it->route, it->fitness);

        if (stagnation >= params.stagnation)
            break;
    }

    return {best_solution, best_value};
}
