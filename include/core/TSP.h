#pragma once
#include <vector>
#include "Graph.h"

class TSP {
public:
    explicit TSP(int size);

    void addGraph(const Graph& g);
    void clearGraph();

    double evaluate(const std::vector<int>& solution) const;

private:
    Graph g;
    std::vector<std::vector<double>> dist;
};
