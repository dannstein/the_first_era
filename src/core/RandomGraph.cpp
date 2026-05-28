#include "core/RandomGraph.h"
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>
#include <vector>

static constexpr float MAP_W = 1672.f;
static constexpr float MAP_H = 941.f;

// Union-Find for connectivity check
struct UF {
    std::vector<int> p;
    explicit UF(int n) : p(n) { std::iota(p.begin(), p.end(), 0); }
    int find(int x) { return p[x] == x ? x : p[x] = find(p[x]); }
    void unite(int a, int b) { p[find(a)] = find(b); }
    bool same(int a, int b) { return find(a) == find(b); }
};

Graph generateRandomGraph(int nodeCount, NodePositions& outPositions, unsigned seed) {
    std::mt19937 rng(seed ? seed : std::random_device{}());

    // --- 1. Grid-based placement for even coverage ---
    // Compute a grid that fits nodeCount cells in the map's aspect ratio.
    int cols = std::max(1, (int)std::ceil(std::sqrt((float)nodeCount * MAP_W / MAP_H)));
    int rows = (int)std::ceil((float)nodeCount / cols);
    float cellW = MAP_W / cols;
    float cellH = MAP_H / rows;

    // Pick nodeCount cells from the cols*rows grid (possibly shuffled to vary layout)
    std::vector<int> cells(cols * rows);
    std::iota(cells.begin(), cells.end(), 0);
    std::shuffle(cells.begin(), cells.end(), rng);
    cells.resize(nodeCount);

    // Place each node with ~80% jitter inside its cell (10% padding each side)
    std::uniform_real_distribution<float> jx(cellW * 0.1f, cellW * 0.9f);
    std::uniform_real_distribution<float> jy(cellH * 0.1f, cellH * 0.9f);

    std::vector<sf::Vector2f> pos(nodeCount);
    for (int i = 0; i < nodeCount; i++) {
        int c = cells[i] % cols;
        int r = cells[i] / cols;
        pos[i] = { c * cellW + jx(rng), r * cellH + jy(rng) };
    }

    // --- 2. Build K-nearest-neighbour edges ---
    int K = std::min(5, nodeCount - 1);

    Graph g(nodeCount);
    for (int i = 0; i < nodeCount; i++)
        g.setNode(i, "Node " + std::to_string(i));

    // Precompute all pairwise Euclidean distances
    auto dist = [&](int a, int b) {
        float dx = pos[a].x - pos[b].x;
        float dy = pos[a].y - pos[b].y;
        return std::sqrt(dx * dx + dy * dy);
    };

    UF uf(nodeCount);

    for (int i = 0; i < nodeCount; i++) {
        std::vector<std::pair<float, int>> nbrs;
        nbrs.reserve(nodeCount - 1);
        for (int j = 0; j < nodeCount; j++) {
            if (j != i) nbrs.push_back({ dist(i, j), j });
        }
        std::sort(nbrs.begin(), nbrs.end());

        for (int k = 0; k < K; k++) {
            int j = nbrs[k].second;
            float d = nbrs[k].first;
            if (!g.hasEdge(i, j)) {
                g.addEdge(i, j, d, 0.0, 0.0);
                g.addEdge(j, i, d, 0.0, 0.0);
                uf.unite(i, j);
            }
        }
    }

    // --- 3. Ensure full connectivity ---
    // For each node not yet connected to node 0's component, find its nearest
    // node in the main component and add a bridge edge.
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < nodeCount; i++) {
            if (uf.same(i, 0)) continue;
            float best = std::numeric_limits<float>::max();
            int   bestJ = -1;
            for (int j = 0; j < nodeCount; j++) {
                if (!uf.same(j, 0)) continue;
                float d = dist(i, j);
                if (d < best) { best = d; bestJ = j; }
            }
            if (bestJ >= 0) {
                g.addEdge(i, bestJ, best, 0.0, 0.0);
                g.addEdge(bestJ, i, best, 0.0, 0.0);
                uf.unite(i, bestJ);
                changed = true;
            }
        }
    }

    // --- 4. Write positions ---
    for (int i = 0; i < nodeCount; i++)
        outPositions.set(i, pos[i]);

    return g;
}
