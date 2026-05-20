# The First Era — Algorithm & Implementation Documentation

This document covers the theoretical foundations, implementation details, and design decisions behind every class and algorithm in this project. The goal is to explain not just *what* the code does, but *why* each decision was made.

---

## Table of Contents

1. [Problem Definition](#1-problem-definition)
2. [Core Data Structures](#2-core-data-structures)
   - [Node](#21-node)
   - [Edge](#22-edge)
   - [Graph](#23-graph)
   - [TSP](#24-tsp)
   - [Randomize](#25-randomize)
3. [Hill Climbing (HC)](#3-hill-climbing-hc)
4. [Hill Climbing with Tries (HCT)](#4-hill-climbing-with-tries-hct)
5. [Simulated Annealing (SA)](#5-simulated-annealing-sa)
6. [Genetic Algorithm (GA)](#6-genetic-algorithm-ga)

---

## 1. Problem Definition

This project applies AI optimization algorithms to a **Traveling Salesman Problem (TSP)** set in a fantasy world map. The goal is to find the lowest-cost route that visits all 50 locations exactly once and returns to the starting point.

The world is divided into 6 regions, each with different biomes that influence travel cost. The cost of traversing an edge is not just distance — it also accounts for danger and terrain difficulty:

```
C = D + 4*P + 3*T
```

Where:
- **D** = Distance
- **P** = Danger (weighted more heavily — threats directly threaten survival)
- **T** = Terrain difficulty (weighted less — slows travel but is manageable)

The map is **sparse**: not every location has a direct connection to every other. This means algorithms cannot naively jump between any two nodes. This was resolved using Floyd-Warshall (see [TSP section](#24-tsp)).

---

## 2. Core Data Structures

### 2.1 Node

**File:** `include/core/Node.h`, `src/core/Node.cpp`

```cpp
struct Node {
    int id;
    std.string name;
};
```

A `Node` represents a location on the map. It holds an integer `id` (0-based index internally, 1-based in output) and a `name` string for human-readable display.

**Why a struct and not a class?**
Nodes are plain data containers with no behaviour. In C++, `struct` is appropriate when the type is purely data — there is no logic to encapsulate, no invariants to protect. Using a class would add unnecessary ceremony.

**Why store the id inside the node?**
When printing a route, we iterate over a `vector<int>` of indices and call `graph.getNode(index)`. Having `id` inside the node means we don't need to pass the index separately to the print function — the node carries its own identity.

---

### 2.2 Edge

**File:** `include/core/Edge.h`, `src/core/Edge.cpp`

```cpp
struct Edge {
    double distance;
    double danger;
    double difficulty;

    double weight() const;
    bool exists() const;
};
```

An `Edge` represents a directed connection between two nodes. It stores three raw attributes and provides two computed methods.

**Why store three separate components instead of just weight?**
The three components (distance, danger, difficulty) may be used independently in the future — for example, to display route statistics or to allow the user to change the weight formula without modifying stored data. Storing only the pre-computed weight would make the data irreversible.

**`weight()` — the cost formula:**
```cpp
double Edge::weight() const {
    return distance + 4.0 * danger + 3.0 * difficulty;
}
```
The multipliers encode the problem's priorities: danger is the most penalised factor (×4) since it directly threatens the traveler's life, while terrain difficulty (×3) slows travel but is more manageable.

**`exists()` — the sentinel pattern:**
```cpp
bool Edge::exists() const {
    return distance >= 0;
}
```
The default constructor sets `distance = -1`. This acts as a sentinel value: a default-constructed edge means "no connection". `exists()` checks for this, allowing the adjacency matrix to represent absent edges without a separate boolean matrix. This keeps the data structure simple and cache-friendly.

**Why `double` for all values?**
The weight formula mixes integer-valued inputs (biome ratings are integers 1–10) but the result of the formula and the Floyd-Warshall distances are real numbers. Using `double` throughout avoids any silent integer truncation.

---

### 2.3 Graph

**File:** `include/core/Graph.h`, `src/core/Graph.cpp`

```cpp
class Graph {
    std::vector<Node> nodes;
    std::vector<std::vector<Edge>> matrix;
    int n;
};
```

The `Graph` class represents the world map as an **adjacency matrix** of `Edge` objects.

**Why an adjacency matrix instead of an adjacency list?**
For this problem, the TSP algorithms need to query the edge between any two nodes in O(1) time — `matrix[i][j]` is a direct array access. An adjacency list would require searching through a list for each lookup. Since the map has only 50 nodes, the memory cost of a 50×50 matrix (2500 edges) is negligible, and the O(1) access pattern is a clear win.

**Why directed edges (both directions stored separately)?**
`addEdge(from, to, ...)` only sets `matrix[from][to]`. The `WorldMap` builder calls it twice (both directions) for undirected connections. This design keeps the `Graph` class general — it makes no assumption about symmetry, which allows future use cases with one-way paths.

**`allPairsShortestPath()` — Floyd-Warshall:**
```cpp
std::vector<std::vector<double>> Graph::allPairsShortestPath() const {
    // initialise with direct edge weights or infinity
    // triple loop: for each intermediate node k, relax all i→j paths
}
```
Floyd-Warshall computes the shortest weighted path between every pair of nodes in O(n³). This is called once when a graph is loaded into `TSP`. The result is a complete 50×50 distance matrix where every entry `dist[i][j]` holds the cheapest cost to travel from node `i` to node `j` through any sequence of intermediate nodes.

**Why is this necessary?**
The world map is **sparse** — most node pairs have no direct connection. Without Floyd-Warshall, evaluating a TSP route would encounter missing edges constantly. The Floyd-Warshall precomputation effectively converts the sparse graph into a complete one, where every cost reflects the actual cheapest path through the map. The TSP algorithms then operate on this complete cost matrix, which is their intended input.

The INF guard in the relaxation step:
```cpp
if (dist[i][k] != INF && dist[k][j] != INF)
    dist[i][j] = std::min(dist[i][j], dist[i][k] + dist[k][j]);
```
prevents infinity + infinity overflow (undefined behaviour for floating-point infinity arithmetic in some compilers).

---

### 2.4 TSP

**File:** `include/core/TSP.h`, `src/core/TSP.cpp`

```cpp
class TSP {
    Graph g;
    std::vector<std::vector<double>> dist;
};
```

`TSP` is the evaluation engine. It stores the graph and its precomputed shortest-path matrix, and exposes a single `evaluate` method used by all algorithms.

**`addGraph` triggers Floyd-Warshall:**
```cpp
void TSP::addGraph(const Graph& newGraph) {
    g = newGraph;
    dist = g.allPairsShortestPath();
}
```
Floyd-Warshall runs exactly once at load time. Every subsequent call to `evaluate` is O(n) — just summing n pre-computed distances.

**`evaluate` — tour cost:**
```cpp
double TSP::evaluate(const std::vector<int>& solution) const {
    double v = 0.0;
    int size = solution.size();
    for (int i = 0; i < size - 1; i++)
        v += dist[solution[i]][solution[i + 1]];
    v += dist[solution[size - 1]][solution[0]]; // return to start
    return v;
}
```
A solution is a permutation of node indices (e.g., `[3, 11, 27, ...]`). The tour cost is the sum of travel costs between consecutive nodes, plus the cost to return from the last node back to the first. This closes the tour, which is the TSP requirement.

**Why pass solutions as `const vector<int>&`?**
All algorithms construct candidate solutions as temporary vectors. Passing by `const` reference avoids copying a 50-element vector on every evaluation call. Since HC/HCT evaluate n*(n-1)/2 ≈ 1225 solutions per step, this matters for performance.

---

### 2.5 Randomize

**File:** `include/core/Randomize.h`, `src/core/Randomize.cpp`

```cpp
class Randomize {
public:
    std::vector<int> randomSolution(int size);
    static int randomNumber(int size);
};
```

**`randomSolution` — Fisher-Yates shuffle:**
```cpp
std::vector<int> Randomize::randomSolution(int size) {
    std::vector<int> result(size);
    std::iota(result.begin(), result.end(), 0);
    std::shuffle(result.begin(), result.end(), std::mt19937{std::random_device{}()});
    return result;
}
```
Generates a valid TSP solution — a random permutation of `[0, 1, ..., size-1]`. The approach:
1. `std::iota` fills the vector with `0, 1, 2, ..., size-1` in O(n)
2. `std::shuffle` with `std::mt19937` applies a Fisher-Yates shuffle in O(n)

This is always O(n) and guaranteed to produce a valid permutation with no duplicates. An earlier implementation used rejection sampling (generate a random number, skip if already used), which degrades to O(n²) as the vector fills. The `iota + shuffle` approach is both faster and simpler.

**Why `std::mt19937`?**
`std::mt19937` (Mersenne Twister) is the standard C++ high-quality random number generator. It has a period of 2¹⁹⁹³⁷-1, passes all standard statistical tests, and is seeded with `std::random_device{}()` which reads from the OS entropy source. This ensures different results across runs.

**`randomNumber` is `static`:**
The method has no state — it creates its own generator internally. Making it `static` reflects this: it belongs to the class conceptually but doesn't need an instance. SA and GA call it as `Randomize::randomNumber(size)` without constructing an object.

---

## 3. Hill Climbing (HC)

**Files:** `include/algorithm/HC.h`, `src/algorithm/HC.cpp`

### Theory

Hill Climbing is a **local search** algorithm. Starting from an initial solution, it repeatedly moves to a better neighbouring solution until no improvement can be found. The analogy is climbing a hill: always step uphill (or in our case, downhill since we minimise cost) until you reach a peak (local optimum).

This is **steepest-descent** hill climbing: at each step, all neighbours are evaluated and the best one is selected — not just the first improvement found.

### Implementation

**`Neighbors` — the swap neighbourhood:**
```cpp
for (int i = 0; i < size - 1; i++) {
    for (int j = i + 1; j < size; j++) {
        std::vector<int> s = solution;
        std::swap(s[i], s[j]);
        double vs = tsp.evaluate(s);
        // track best
    }
}
```
For a solution of size n, this evaluates all n*(n-1)/2 pairwise swaps. For n=50, that is 1225 evaluations per step. Each swap produces a new candidate route and evaluates its cost.

**`Hill` — the main loop:**
```cpp
while (true) {
    auto [new_solution, new_value] = Neighbors(current, tsp);
    if (new_value < current_value) {
        current = new_solution;
        current_value = new_value;
    } else {
        return {current, current_value};
    }
}
```
If the best neighbour is better than the current solution, move to it and repeat. If not, we have reached a local optimum — return immediately.

### Limitations

- **Local optima:** HC stops at the first local optimum it reaches, even if the global optimum is far away. The quality of the result depends entirely on the starting solution.
- **No backtracking:** once a worse direction is rejected, HC never reconsiders it.
- **Starting-point sensitivity:** a bad random initial solution will likely end in a bad local optimum.

---

## 4. Hill Climbing with Tries (HCT)

**Files:** `include/algorithm/HCT.h`, `src/algorithm/HCT.cpp`

### Theory

HCT is a direct extension of HC that addresses its most critical limitation: stopping immediately at the first local optimum. HCT allows the search to continue for up to `t_max` additional steps even when the best neighbour is worse than the current solution, in the hope of escaping a "valley" in the search landscape.

### Implementation

The key change from HC is in the main loop:

```cpp
std::vector<int> best_solution = current;
double best_value = current_value;
int t = 0;

while (true) {
    auto [new_solution, new_value] = Neighbors(current, tsp);

    if (new_value < current_value) {
        current = new_solution;
        current_value = new_value;
        t = 0;                          // improvement found: reset try counter
        if (current_value < best_value) {
            best_solution = current;
            best_value = current_value;
        }
    } else if (t < t_max) {
        current = new_solution;         // accept worse: move anyway
        current_value = new_value;
        t++;
    } else {
        return {best_solution, best_value};
    }
}
```

**Why track `best_solution` separately from `current`?**
When HCT accepts a worse move, `current` may degrade. After wandering through worse solutions, the algorithm might overshoot the local optimum and end up in a worse place than where it started from. By maintaining `best_solution` separately and only updating it on genuine improvements, HCT always returns the best value it ever visited — not necessarily where it ended up.

**Why reset `t = 0` on improvement?**
If the algorithm finds an improvement after accepting some worse moves, it has successfully escaped a valley. The try counter is reset to give the search a fresh budget from the new (better) position.

**The `Neighbors` function is identical to HC** — the same full swap neighbourhood is explored at every step. The only difference is in how the best neighbour is accepted or rejected in the outer loop.

---

## 5. Simulated Annealing (SA)

**Files:** `include/algorithm/SA.h`, `src/algorithm/SA.cpp`

### Theory

Simulated Annealing is inspired by the metallurgical process of annealing: slowly cooling a material so atoms settle into a low-energy crystalline structure. In optimisation terms, it is a probabilistic local search that can accept worse solutions, with the probability of doing so decreasing over time as the "temperature" drops.

The key insight over HC: accepting worse moves occasionally allows the algorithm to escape local optima. As temperature decreases, the algorithm becomes increasingly selective, eventually behaving like hill climbing.

The acceptance probability for a worse solution follows the **Boltzmann distribution**:

```
P(accept) = e^(-Δ / T)
```

Where:
- **Δ** = cost increase (how much worse the new solution is)
- **T** = current temperature

At high temperature, even very bad moves have a reasonable chance of being accepted. As T approaches 0, the probability approaches 0, and only improvements are accepted.

### Parameters

```cpp
static constexpr double TI  = 1000.0; // Initial temperature
static constexpr double TF  = 0.001;  // Final temperature
static constexpr double FR  = 0.995;  // Cooling rate (factor per step)
```

- **TI = 1000:** High enough that many moves are accepted early, allowing broad exploration. With typical tour cost differences in the hundreds, `e^(-100/1000) ≈ 0.90` — a 100-point worsening has a 90% chance of acceptance at the start.
- **TF = 0.001:** Near-zero. At this temperature, `e^(-1/0.001) ≈ 0` — the algorithm effectively only accepts improvements.
- **FR = 0.995:** Slow cooling. Total iterations: `log(TF/TI) / log(FR) ≈ 13,800`. A slower cooling rate gives the algorithm more time to explore at each temperature level.

**Why TF = 0.001 and not 0.1?**
At TF = 0.1, the algorithm still has a non-trivial acceptance probability for small worsenings at termination, meaning it has not fully "frozen". 0.001 ensures the system is effectively frozen before stopping.

### Implementation

**`Neighbor` — single random swap:**
```cpp
int i = Randomize::randomNumber(size);
int j = Randomize::randomNumber(size);
while (j == i) j = Randomize::randomNumber(size);
std::vector<int> s = solution;
std::swap(s[i], s[j]);
return {s, tsp.evaluate(s)};
```
Unlike HC which evaluates all neighbours, SA evaluates **one random neighbour** per step. This is intentional: SA's strength is in its probabilistic acceptance, not exhaustive search. Evaluating a single random neighbour per temperature step keeps the per-iteration cost O(n) instead of O(n²).

**`Annealing` — the main loop:**
```cpp
while (temperature > TF) {
    auto [new_solution, new_value] = Neighbor(current, tsp);

    if (new_value < current_value) {
        // always accept improvements
        current = new_solution;
        current_value = new_value;
        if (current_value < best_value) { ... }
    } else {
        double d = new_value - current_value;
        if (prob(gen) < std::exp(-d / temperature)) {
            // probabilistically accept worse solution
            current = new_solution;
            current_value = new_value;
        }
    }

    temperature *= FR; // cool down every step
}
```

**Why track `best_solution` separately?**
SA's random walk means the current solution at the end of the run may be worse than the best solution ever visited during the run. The algorithm can wander into worse territory near the end as temperature drops and acceptance becomes rare but not zero. Tracking the best-ever solution guarantees the returned result is the global best found.

**Why `std::uniform_real_distribution<double>(0.0, 1.0)` for acceptance?**
The Boltzmann acceptance probability `e^(-Δ/T)` is a real number in (0, 1). To determine whether to accept, we draw a uniform random number in [0, 1] and accept if it is less than the probability. This is the standard Monte Carlo acceptance criterion and is mathematically correct.

---

## 6. Genetic Algorithm (GA)

**Files:** `include/algorithm/GA.h`, `src/algorithm/GA.cpp`

### Theory

Genetic Algorithms are population-based search methods inspired by natural evolution. Instead of a single solution, GA maintains a **population** of candidate solutions. Each generation, solutions are selected based on fitness (quality), combined through crossover (recombination), and occasionally mutated. Over generations, the population evolves toward better solutions.

The analogy: solutions are "chromosomes". Good chromosomes reproduce more often, passing their genetic material to offspring. Poor chromosomes are gradually eliminated. Mutation introduces occasional random variation to prevent stagnation.

For TSP, a chromosome is a **permutation** of node indices — a complete visiting order.

### The `Individual` Struct

```cpp
struct Individual {
    std::vector<int> route;
    double fitness;
};
```

`route` and `fitness` are bundled together. This design decision prevents the population vector and fitness vector from falling out of sync (a common bug when they are stored separately). Fitness is computed **exactly once** when an `Individual` is created — at initialisation, after crossover+mutation, or when a random individual is injected. It is never recomputed unnecessarily.

### Parameters

```cpp
static constexpr int    POP_SIZE         = 200;
static constexpr int    GENERATIONS      = 500;
static constexpr double MUTATION_RATE    = 0.08;
static constexpr int    TOURNAMENT_SIZE  = 3;
static constexpr double GI               = 0.1;  // Generation Interval (elite %)
static constexpr double BR               = 0.8;  // Breeding Rate (offspring %)
static constexpr int    STAGNATION_LIMIT = 75;
```

### Population Initialisation

```cpp
for (int i = 0; i < POP_SIZE; i++) {
    std::vector<int> route = Randomize().randomSolution(size);
    pop.push_back({route, tsp.evaluate(route)});
}
```

Each of the 200 initial individuals is a random permutation generated by `Randomize::randomSolution`. Fitness is evaluated immediately at creation. Starting with a diverse random population is critical — a biased initial population can permanently limit what the GA discovers.

### Selection — Tournament

```cpp
Individual GA::selection(const Population& pop, std::mt19937& gen) {
    std::uniform_int_distribution<int> distr(0, pop.size() - 1);
    int best = distr(gen);
    for (int i = 1; i < TOURNAMENT_SIZE; i++) {
        int candidate = distr(gen);
        if (pop[candidate].fitness < pop[best].fitness)
            best = candidate;
    }
    return pop[best];
}
```

Tournament selection picks `TOURNAMENT_SIZE` (3) individuals at random and returns the best among them. This approach:
- Does not require sorting the entire population
- Gives weaker individuals a non-zero chance of being selected (preserving diversity)
- Is tunable: larger tournament = stronger selection pressure = faster convergence but less diversity

**Why tournament size 3?**
With POP_SIZE=200 and tournament size 3, we sample 1.5% of the population per selection. A larger tournament (5+) was tested and caused premature convergence — the same top individuals dominated reproduction, reducing diversity and worsening results. Size 3 maintains healthy exploration.

### Crossover — Order Crossover (OX)

Standard crossover operators (single-point, two-point) cannot be applied directly to TSP permutations because they produce invalid routes with duplicate nodes. Order Crossover (OX) is specifically designed for permutation problems.

**How OX works:**
1. Select two random cut points `start` and `end`
2. Copy the segment `[start..end]` from parent 1 directly into the child
3. Fill the remaining positions with genes from parent 2, in the order they appear in parent 2, starting from position `(end+1)`, skipping any gene already present in the child

```cpp
// Step 1: copy segment from p1
for (int i = start; i <= end; i++) {
    child[i] = p1[i];
    used[p1[i]] = true;
}

// Step 2: fill from p2 in order, skipping duplicates
int pos = (end + 1) % size;
for (int i = 0; i < size; i++) {
    int gene = p2[(end + 1 + i) % size];
    if (!used[gene]) {
        child[pos] = gene;
        used[gene] = true;
        pos = (pos + 1) % size;
    }
}
```

The `used[]` boolean array tracks which nodes are already in the child, guaranteeing no duplicates. The result is always a valid permutation containing every node exactly once.

**Why OX specifically?**
OX preserves the **relative order** of cities from parent 2, which tends to preserve good sub-sequences (subsequences that represent efficient local routes). This is more meaningful for TSP than position-based crossover operators.

### Mutation — 2-opt Reversal

```cpp
void GA::mutate(std::vector<int>& solution, std::mt19937& gen) {
    std::uniform_real_distribution<double> prob(0.0, 1.0);
    if (prob(gen) < MUTATION_RATE) {
        std::uniform_int_distribution<int> distr(0, solution.size() - 1);
        int i = distr(gen);
        int j = distr(gen);
        if (i > j) std::swap(i, j);
        std::reverse(solution.begin() + i, solution.begin() + j + 1);
    }
}
```

With probability `MUTATION_RATE` (8%), a random segment `[i..j]` of the route is reversed. This is equivalent to a **2-opt move**: it removes the two edges connecting to the endpoints of the segment and reconnects them in the only other valid way.

**Why reversal instead of simple swap?**
A simple swap exchanges two isolated cities, which is a relatively unstructured change. A reversal restructures the order of an entire sub-sequence, which directly explores the **2-opt neighbourhood** — one of the most well-studied and effective local search structures for TSP. In practice, 2-opt reversal mutation produces noticeably better results than swap mutation for routing problems.

**Why 8% mutation rate?**
Too low (1–2%): the population homogenises quickly, and the algorithm converges prematurely to local optima. Too high (20%+): solutions are essentially re-randomised each generation, destroying useful structure from crossover. 8% balances exploration and exploitation for a 50-node problem.

### Generational Replacement — GI / BR Structure

Each generation, the new population is assembled from three sources:

```
new_pop = elite (GI%) + offspring (BR%) + random (remainder%)
```

| Source | % | Count (POP=200) | Purpose |
|---|---|---|---|
| Elite (GI) | 10% | 20 | Preserve the best — prevents regression |
| Offspring (BR) | 80% | 160 | Breed new solutions from good parents |
| Random | 10% | 20 | Inject fresh diversity, prevent stagnation |

**Why elitism (GI)?**
Without elitism, a good solution found in generation N could be lost in generation N+1 if its offspring are worse. Elitism guarantees monotonic improvement of the best known solution.

**Why random injection?**
After many generations, the population tends to converge: most individuals become similar. Random injection continuously introduces fresh genetic material, preventing the GA from becoming trapped in a single region of the search space.

**Elite selection via sorting:**
```cpp
std::sort(pop.begin(), pop.end(),
    [](const Individual& a, const Individual& b) { return a.fitness < b.fitness; });
for (int i = 0; i < elite_count; i++)
    new_pop.push_back(pop[i]);
```
The population is sorted by fitness each generation. The top `elite_count` individuals are taken directly. This is O(n log n) but with n=200 it is negligible, and it simplifies the elite selection logic considerably compared to `partial_sort` with an index vector.

### Early Stopping — Stagnation Detection

```cpp
if (it->fitness < best_value) {
    best_value    = it->fitness;
    best_solution = it->route;
    stagnation    = 0;
} else {
    stagnation++;
}

if (stagnation >= STAGNATION_LIMIT)
    break;
```

If the global best solution does not improve for `STAGNATION_LIMIT` (75) consecutive generations, the algorithm stops early. This avoids wasting computation on generations that are no longer productive. In practice, once the GA stagnates for 75 generations, it is extremely unlikely to escape — the population has converged and further iteration yields no benefit.

**Why 75?**
- Too low (20–30): the algorithm may stop before it has had time to exploit a recently discovered promising region.
- Too high (150+): wastes computation after genuine convergence.
- 75 with 500 max generations means the algorithm tolerates stagnation for up to 15% of its budget before giving up.

### The Full Generational Loop

```
for each generation:
  1. Sort population by fitness
  2. Copy top GI% directly to new population (elitism)
  3. For BR% of population:
       a. Select two parents via tournament
       b. Apply OX crossover → child route
       c. Apply 2-opt reversal mutation with probability 8%
       d. Evaluate child fitness (once)
       e. Add to new population
  4. Generate random% new individuals (inject diversity)
  5. Replace population
  6. Find generation best, update global best
  7. Increment or reset stagnation counter
  8. If stagnation >= 75, stop early
```

### Observed Performance

On the 50-node world map with Floyd-Warshall shortest paths:
- Random initial solution: ~12,000 cost
- HC/HCT result: 2,900–4,000 (depends on starting point)
- SA result: ~2,700
- GA result: **2,500–3,000**, rarely exceeding 3,000

The GA outperforms the others because it searches many regions of the solution space simultaneously (population diversity) and combines useful sub-sequences from multiple good solutions (crossover), rather than improving a single solution from a single starting point.
