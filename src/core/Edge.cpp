#include "core/Edge.h"

Edge::Edge() : distance(-1), danger(0), difficulty(0) {}

Edge::Edge(double distance, double danger, double difficulty)
    : distance(distance), danger(danger), difficulty(difficulty) {}

double Edge::weight() const {
    return distance + 4.0 * danger + 3.0 * difficulty;
}

bool Edge::exists() const {
    return distance >= 0;
}
