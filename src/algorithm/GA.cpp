#include "algorithm/GA.h"
#include "core/Randomize.h"
#include <algorithm>
#include <numeric>

GA::Population GA::initPopulation(int size) {
    Population pop;
    pop.reserve(POP_SIZE);
    for (int i = 0; i < POP_SIZE; i++)
        pop.push_back(Randomize().randomSolution(size));
    return pop;
}

std::vector<int> GA::selection(const Population& pop, const std::vector<double>& fitness, std::mt19937& gen) {
    std::uniform_int_distribution<int> distr(0, (int)pop.size() - 1);
    int best = distr(gen);
    for (int i = 1; i < TOURNAMENT_SIZE; i++) {
        int candidate = distr(gen);
        if (fitness[candidate] < fitness[best])
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
        while (j == i) j = distr(gen);
        std::swap(solution[i], solution[j]);
    }
}

std::pair<std::vector<int>, double> GA::run(const TSP& tsp, int size) {
    std::mt19937 gen(std::random_device{}());

    Population pop = initPopulation(size);

    std::vector<double> fitness(POP_SIZE);
    for (int i = 0; i < POP_SIZE; i++)
        fitness[i] = tsp.evaluate(pop[i]);

    int best_idx = std::min_element(fitness.begin(), fitness.end()) - fitness.begin();
    std::vector<int> best_solution = pop[best_idx];
    double best_value = fitness[best_idx];

    for (int g = 0; g < GENERATIONS; g++) {
        Population new_pop;
        new_pop.reserve(POP_SIZE);

        // Elitism: carry the top ELITE_SIZE solutions unchanged
        std::vector<int> indices(POP_SIZE);
        std::iota(indices.begin(), indices.end(), 0);
        std::partial_sort(indices.begin(), indices.begin() + ELITE_SIZE, indices.end(),
            [&](int a, int b) { return fitness[a] < fitness[b]; });
        for (int i = 0; i < ELITE_SIZE; i++)
            new_pop.push_back(pop[indices[i]]);

        while ((int)new_pop.size() < POP_SIZE) {
            std::vector<int> p1 = selection(pop, fitness, gen);
            std::vector<int> p2 = selection(pop, fitness, gen);

            std::vector<int> child = crossover(p1, p2, gen);

            mutate(child, gen);
            new_pop.push_back(child);
        }

        pop = std::move(new_pop);

        for (int i = 0; i < POP_SIZE; i++)
            fitness[i] = tsp.evaluate(pop[i]);

        for (int i = 0; i < POP_SIZE; i++) {
            if (fitness[i] < best_value) {
                best_value    = fitness[i];
                best_solution = pop[i];
            }
        }
    }

    return {best_solution, best_value};
}
