#pragma once
#include "core/Graph.h"
#include "gui/NodePositions.h"

// Generates a random connected graph with `nodeCount` nodes.
// Positions are grid-distributed across the map for good visual coverage.
// Returns the Graph; writes image-space positions into outPositions.
Graph generateRandomGraph(int nodeCount, NodePositions& outPositions, unsigned seed = 0);
