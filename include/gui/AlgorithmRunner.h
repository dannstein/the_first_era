#pragma once
#include <vector>
#include "core/TSP.h"

struct AnimFrame {
    std::vector<int> route;
    double           cost;
    bool             isBest;
};

class AlgorithmRunner {
public:
    void run(int algoChoice, int hctTMax,
             const TSP& tsp, int nodeCount,
             int maxFrames = 250);

    const std::vector<AnimFrame>& frames()    const;
    const std::vector<int>&       bestRoute() const;
    double                        bestCost()  const;

private:
    std::vector<AnimFrame> m_frames;
    std::vector<int>       m_best;
    double                 m_bestCost = 0.0;
};
