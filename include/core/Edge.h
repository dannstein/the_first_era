#pragma once

struct Edge {
    double distance;
    double danger;
    double difficulty;

    Edge();
    Edge(double distance, double danger, double difficulty);

    double weight() const;
    bool exists() const;
};

static const Edge NO_EDGE = Edge();
