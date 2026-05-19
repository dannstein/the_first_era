#include "core/TSP.h"

TSP::TSP(int size) : g(size) {}

void TSP::addGraph(const Graph& newGraph) {
    g = newGraph;
    dist = g.allPairsShortestPath();
}

void TSP::clearGraph() {
    g = Graph(g.size());
    dist.clear();
}

double TSP::evaluate(const std::vector<int>& solution) const {
    double v = 0.0;
    int size = solution.size();

    for (int i = 0; i < size - 1; i++)
        v += dist[solution[i]][solution[i + 1]];

    v += dist[solution[size - 1]][solution[0]];

    return v;
}
