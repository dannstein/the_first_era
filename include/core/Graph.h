#pragma once
#include <vector>
#include <string>
#include "Node.h"
#include "Edge.h"

class Graph {
public:
    explicit Graph(int numNodes);

    void setNode(int id, const std::string& name);
    void addEdge(int from, int to, double distance, double danger, double difficulty);
    void removeEdge(int from, int to);

    const Edge& getEdge(int from, int to) const;
    const Node& getNode(int id) const;
    int size() const;
    bool hasEdge(int from, int to) const;

    std::vector<std::vector<double>> allPairsShortestPath() const;

private:
    std::vector<Node> nodes;
    std::vector<std::vector<Edge>> matrix;
    int n;
};
