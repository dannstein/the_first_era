#include "core/Graph.h"
#include <stdexcept>
#include <limits>

Graph::Graph(int numNodes)
    : n(numNodes),
      nodes(numNodes),
      matrix(numNodes, std::vector<Edge>(numNodes)) {}

void Graph::setNode(int id, const std::string& name) {
    nodes[id] = Node(id, name);
}

void Graph::addEdge(int from, int to, double distance, double danger, double difficulty) {
    matrix[from][to] = Edge(distance, danger, difficulty);
}

void Graph::removeEdge(int from, int to) {
    matrix[from][to] = Edge();
}

const Edge& Graph::getEdge(int from, int to) const {
    return matrix[from][to];
}

const Node& Graph::getNode(int id) const {
    return nodes[id];
}

int Graph::size() const {
    return n;
}

bool Graph::hasEdge(int from, int to) const {
    return matrix[from][to].exists();
}

std::vector<std::vector<double>> Graph::allPairsShortestPath() const {
    const double INF = std::numeric_limits<double>::infinity();
    std::vector<std::vector<double>> dist(n, std::vector<double>(n, INF));

    for (int i = 0; i < n; i++) {
        dist[i][i] = 0;
        for (int j = 0; j < n; j++) {
            if (matrix[i][j].exists())
                dist[i][j] = matrix[i][j].weight();
        }
    }

    for (int k = 0; k < n; k++)
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (dist[i][k] != INF && dist[k][j] != INF)
                    dist[i][j] = std::min(dist[i][j], dist[i][k] + dist[k][j]);

    return dist;
}
