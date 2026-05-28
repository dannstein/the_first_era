# Algorithm Evolution — The First Era

This document traces every version of each algorithm from its first implementation to the current state, explaining what was wrong in each iteration and why the changes were made.

---

## 1. Hill Climbing (HC)

### Version 1 — First implementation (`feat? added all the local search methods`)

```cpp
std::pair<std::vector<int>, double> HC::Hill(
    const std::vector<int>& solution, double value, const TSP& tsp)

std::pair<std::vector<int>, double> HC::Neighbors(
    const std::vector<int>& solution, const TSP& tsp)
```

The algorithm was already conceptually correct from the start. `Neighbors` evaluated all n·(n−1)/2 pairwise swaps (for n=50: 1,225 per step) and returned the single best. `Hill` moved to that best neighbor if it was an improvement, and returned if not — a textbook steepest-descent local search.

**Problem:** No concept of a fixed start node. The swap loop started at index 0, so position 0 — the starting city — could be moved anywhere in the route. When the city that should be "first" gets shuffled into the middle, the tour is still valid but the semantics are broken for the fixed-start use case.

---

### Version 2 — Current (`feat: added user interface`)

```cpp
std::pair<std::vector<int>, double> HC::Hill(
    const std::vector<int>& solution, double value,
    const TSP& tsp, int fixedStart = -1, AlgoCallback cb = nullptr)
```

Two additions:

- **Fixed start support:** `Neighbors` now accepts `fixedStart`. When it is `>= 0`, the swap loop begins at `startIdx = 1`, leaving position 0 pinned. The algorithm still evaluates all valid swaps — it just never touches the first slot.
- **Animation callback:** Each time the search improves, `cb(current, cost)` is fired. The GUI collects these snapshots into frames for the animation. Passing `nullptr` (the default) has zero cost — it is the same binary as before for non-GUI use.

The core logic is unchanged: still steepest-descent, still stops at the first local optimum.

---

## 2. Hill Climbing with Tries (HCT)

### Version 1 — First implementation (`feat? added all the local search methods`)

```cpp
std::pair<std::vector<int>, double> HCT::Hill(
    const std::vector<int>& solution, double value, const TSP& tsp, int t_max)
```

The intent was to let the search survive a local optimum by accepting worsening moves for up to `t_max` tries:

```cpp
if (new_value < current_value) {
    // improvement: move and reset counter
    t = 0;
} else if (t < t_max) {
    current = new_solution;   // accept worse
    current_value = new_value;
    t++;
} else {
    return {best_solution, best_value};
}
```

**Bug — deterministic cycling:** `Neighbors` is a pure function. Given the same input solution it always returns the exact same best-swap neighbor. When stuck at a local minimum L:

1. `Neighbors(L)` returns N, where `cost(N) >= cost(L)` (we are at a local min, so no swap helps)
2. The `else if` branch accepts N
3. `Neighbors(N)` often returns L (L is frequently the best neighbor of N)
4. The algorithm cycles: L → N → L → N → ... until `t_max` is exhausted, then returns

The `t_max` counter only controlled how long this deterministic loop ran, not how many genuinely different starting points were explored. The algorithm appeared to "try" but was really just oscillating between the same 2–3 states.

---

### Version 2 — Intermediate (GUI integration, same algorithmic bug)

```cpp
std::pair<std::vector<int>, double> HCT::Hill(
    const std::vector<int>& solution, double value,
    const TSP& tsp, int t_max, int fixedStart = -1, AlgoCallback cb = nullptr)
```

The fixed-start parameter and animation callback were added (same as HC v2), but the cycling bug in the escape logic was still present.

---

### Version 3 — Current (random restart, this session)

```cpp
std::pair<std::vector<int>, double> HCT::Hill(
    const std::vector<int>& solution, double value,
    const TSP& tsp, int t_max, int nodeCount,
    int fixedStart = -1, AlgoCallback cb = nullptr)
```

The escape logic was completely replaced with **random-restart hill climbing**:

```cpp
// Run HC to local optimum from any starting point
auto localSearch = [&](std::vector<int> cur, double curVal) {
    while (true) {
        auto [nbr, nbrVal] = Neighbors(cur, tsp, fixedStart);
        if (nbrVal < curVal) { cur = nbr; curVal = nbrVal; if (cb) cb(cur, curVal); }
        else return {cur, curVal};
    }
};

// First run from the provided initial solution
auto [best, bestVal] = localSearch(solution, value);

// t_max additional runs, each from a fresh random start
Randomize rng;
for (int t = 0; t < t_max; t++) {
    auto restart = rng.randomSolution(nodeCount, fixedStart);
    auto [candidate, candidateVal] = localSearch(restart, tsp.evaluate(restart));
    if (candidateVal < bestVal) { best = candidate; bestVal = candidateVal; }
}

return {best, bestVal};
```

Each "try" is now a genuinely independent run from a random starting solution. Since the starting point changes every time, the local optimum found also changes. After `t_max + 1` independent HC runs, the algorithm returns the best result found across all of them. This correctly implements "Hill Climbing with Tries" and eliminates the cycling problem entirely.

**Why `nodeCount` is now a parameter:** `Randomize::randomSolution` needs to know how many nodes exist. This cannot be inferred from the initial `solution` alone for all use cases, so it is passed explicitly.

---

## 3. Simulated Annealing (SA)

### Version 1 — First implementation (`feat? added all the local search methods`)

```cpp
std::pair<std::vector<int>, double> SA::Annealing(
    const std::vector<int>& solution, double value, const TSP& tsp)
```

Parameters were hardcoded as `static constexpr` inside the class header:

```cpp
static constexpr double TI = 1000.0;
static constexpr double TF = 0.001;
static constexpr double FR = 0.995;
```

The `Neighbor` function picked two random positions and swapped them (no fixed-start awareness). The main loop:

```cpp
while (temperature > TF) {
    auto [new_solution, new_value] = Neighbor(current, tsp);

    if (new_value < current_value) {
        // accept improvement
    } else {
        double d = new_value - current_value;
        if (prob(gen) < std::exp(-d / temperature))
            current = new_solution;  // probabilistic acceptance
    }

    temperature *= FR;   // one evaluation, then cool
}
```

**Bug — insufficient iterations:** With TI=1000, TF=0.001, FR=0.995, the temperature loop runs exactly:

```
iterations = log(TF / TI) / log(FR) = log(10⁻⁶) / log(0.995) ≈ 2,755 steps
```

SA did one random-swap evaluation per temperature step — **~2,755 total evaluations.** Meanwhile HC, using the exhaustive swap neighborhood, evaluates 1,225 neighbors per step and typically converges in 10–20 steps — **~12,000–24,000 total evaluations.** SA was ~6–10× less thorough than HC, so despite its probabilistic escape mechanism it reliably produced worse results. The escape mechanism only helps if the algorithm has enough iterations to actually wander out of a local optimum and re-converge somewhere better.

**No fixed-start support:** The random index generation used `Randomize::randomNumber(size)` with no offset, so the first position could be swapped just like any other.

---

### Version 2 — Intermediate (GUI integration, same iteration bug)

```cpp
std::pair<std::vector<int>, double> SA::Annealing(
    const std::vector<int>& solution, double value,
    const TSP& tsp, int fixedStart = -1,
    AlgoCallback cb = nullptr, SAParams params = {})
```

A new `SAParams` struct was introduced so the user could configure TI, TF, and FR at runtime through the parameter config screen:

```cpp
struct SAParams {
    double TI = 1000.0;
    double TF = 0.001;
    double FR = 0.995;
};
```

Fixed-start support was added to `Neighbor` (swapping is restricted to indices `[1, size-1]` when `fixedStart >= 0`). The animation callback was wired in.

The ~2,755 iteration ceiling was still present — changing FR through the UI made it better or worse but the structural problem (1 evaluation per temperature step) remained.

---

### Version 3 — Current (inner loop, this session)

The `Annealing` body gained one additional line and a wrapping `for` loop:

```cpp
int n = (int)solution.size();

while (temperature > params.TF) {
    for (int k = 0; k < n; k++) {         // ← inner loop: n evaluations per temperature
        auto [new_solution, new_value] = Neighbor(current, tsp, fixedStart);
        // acceptance logic unchanged
        if (cb) cb(current, current_value);
    }
    temperature *= params.FR;              // ← cooling moved outside the inner loop
}
```

**Effect:** Total evaluations = n × 2,755. For n=50 nodes → ~137,750 evaluations. This is standard SA practice: run a "sweep" of n random moves at each temperature level before cooling. The cost is O(n) per inner iteration (one swap + evaluate), so the total runtime is O(n² × temperature_steps) — identical in structure to HC but producing far more exploration.

SA now reliably produces lower medians than HC because:
1. It has comparable computational effort to HC
2. It can escape local optima that HC cannot, via probabilistic acceptance at high temperature
3. Best-solution tracking (`best_solution` separate from `current`) ensures the return value is the actual best ever seen, not where the random walk ended up

No signature or parameter changes were needed — `solution.size()` was already available inside `Annealing`.

---

## 4. Genetic Algorithm (GA)

### Version 1 — First implementation (`feat? added all the local search methods`)

**Data model:** `Population` was `std::vector<std::vector<int>>` — a flat list of routes. Fitness was stored in a separate `std::vector<double>`. These two vectors had to stay in sync manually, which was a latent source of bugs.

**Parameters (hardcoded):**
```cpp
static constexpr int    POP_SIZE        = 200;
static constexpr int    GENERATIONS     = 1000;
static constexpr double MUTATION_RATE   = 0.07;
static constexpr int    TOURNAMENT_SIZE = 5;
static constexpr int    ELITE_SIZE      = 2;   // fixed count, not a fraction
```

**Generational structure:** `ELITE_SIZE = 2` individuals carried over unchanged; the rest were produced by selection + crossover + mutation. No random injection, no stagnation detection.

**Mutation — simple swap:**
```cpp
int i = distr(gen), j = distr(gen);
while (j == i) j = distr(gen);
std::swap(solution[i], solution[j]);
```
Swapped two random positions. This is structurally valid (produces a permutation) but a relatively weak perturbation for TSP — it disconnects two isolated cities from their neighbors without restructuring any sub-path.

**Problems:**
- The parallel `pop`/`fitness` vectors were fragile — any operation that reordered one without the other would silently corrupt results.
- `ELITE_SIZE = 2` out of 200 meant 99% of each generation was discarded. Good solutions beyond the top 2 were lost.
- `TOURNAMENT_SIZE = 5` created high selection pressure: the same top individuals dominated reproduction repeatedly, causing premature convergence.
- `GENERATIONS = 1000` with no early stopping wasted computation after the population had converged.
- No random injection meant that once population diversity collapsed, the algorithm stagnated permanently.
- Simple swap mutation was less effective for TSP than neighborhood-aware mutations.

---

### Version 2 — Restructuring (`changed GA parameters and implemented 2-opt move (reversal) mutation`)

**Data model — `Individual` struct:**
```cpp
struct Individual {
    std::vector<int> route;
    double fitness;
};
```
Route and fitness are now bundled together. This makes it impossible for the two to fall out of sync. Fitness is computed exactly once — when an `Individual` is created — and never recomputed unless the route changes.

**Generational structure — GI/BR/Random:**
```
new_pop = elite (GI=10%) + offspring (BR=80%) + random (10%)
```

| Component | Count (POP=200) | Purpose |
|---|---|---|
| Elite (GI=10%) | 20 | Keep the best solutions — monotonic improvement guarantee |
| Offspring (BR=80%) | 160 | Selection + crossover + mutation |
| Random (10%) | 20 | Fresh individuals injected every generation |

Compared to v1: the elite fraction went from 2 individuals to 20, preventing regression. Random injection was added to continuously replenish diversity as the population converges.

**Stagnation detection:**
```cpp
if (it->fitness < best_value) { stagnation = 0; }
else                          { stagnation++;    }
if (stagnation >= STAGNATION_LIMIT) break;
```
`STAGNATION_LIMIT = 75` — if the global best does not improve for 75 consecutive generations, the algorithm stops. Combined with `GENERATIONS = 500` (reduced from 1000), this prevents wasted computation.

**Mutation — 2-opt reversal:**
```cpp
if (i > j) std::swap(i, j);
std::reverse(solution.begin() + i, solution.begin() + j + 1);
```
Instead of swapping two isolated nodes, the segment `[i..j]` is reversed. This is a **2-opt move**: it removes two edges and reconnects the route in the only other valid way. For TSP, this directly explores a much more meaningful neighborhood — reversals tend to fix "crossing" paths that are a common inefficiency. Results improved noticeably over simple swap.

**Parameters updated:**
- `GENERATIONS`: 1000 → 500 (stagnation covers the rest)
- `TOURNAMENT_SIZE`: 5 → 3 (less selection pressure, better diversity)
- `MUTATION_RATE`: 0.07 → 0.08 (slightly more variation)
- `ELITE_SIZE` (fixed int) replaced by `GI = 0.1` (fraction, scales with population)

---

### Version 3 — Current (runtime params + fixed start + callback, GUI integration)

```cpp
std::pair<std::vector<int>, double> GA::run(
    const TSP& tsp, int size, int fixedStart,
    AlgoCallback cb, GAParams params)
```

All static constexpr parameters moved to `GAParams`:

```cpp
struct GAParams {
    int    popSize        = 200;
    int    generations    = 500;
    double mutationRate   = 0.08;
    int    tournamentSize = 3;
    double gi             = 0.1;
    double br             = 0.8;
    int    stagnation     = 75;
};
```

This allows the user to configure the GA through the parameter config screen without recompiling.

**Fixed-start crossover:** When `fixedStart >= 0`, the Order Crossover operates only on the sub-range `[1, size-1]`, leaving index 0 pinned. The `used[]` array is initialized with `fixedStart` already marked, ensuring it is never placed elsewhere.

**Fixed-start mutation:** The reversal index range begins at `low = (fixedStart >= 0) ? 1 : 0`, so position 0 is never included in a reversal segment.

**Animation callback:** At the end of each generation, `cb(best_in_generation.route, best_in_generation.fitness)` is called. The GUI shows the evolution of the best solution across generations, not just the final result. Passing `nullptr` (default) has no cost.

---

## Summary Table

| Algorithm | Version | Key Issue | Fix |
|---|---|---|---|
| HC | v1 | No fixed start — swaps position 0 | Added `fixedStart` offset in swap loop |
| HC | v2 (current) | — | Added callback for animation |
| HCT | v1 | Deterministic cycling: `Neighbors` is pure, same input → same escape, creates L→N→L loop | — |
| HCT | v2 | Same cycling bug, fixed-start added | — |
| HCT | v3 (current) | — | Replaced escape logic with random restart: each try runs HC from a fresh random solution |
| SA | v1 | ~2,755 total evaluations vs HC's ~12,000–24,000; hardcoded params; no fixed start | — |
| SA | v2 | ~2,755 total evaluations (runtime params added but structural problem remains) | — |
| SA | v3 (current) | — | Added inner loop of `n` evaluations per temperature step: ~137,750 total |
| GA | v1 | Parallel route/fitness vectors (fragile), swap mutation, no stagnation, 1000 gens, `ELITE_SIZE=2` | — |
| GA | v2 | — | `Individual` struct, 2-opt reversal, GI/BR/random structure, stagnation detection, 500 gens |
| GA | v3 (current) | — | Runtime `GAParams`, fixed-start-aware OX crossover and reversal mutation, generation callback |
