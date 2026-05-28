#pragma once
#include <vector>
#include "core/TSP.h"
#include "algorithm/AlgoParams.h"

struct AnimFrame {
    std::vector<int> route;
    double           cost;
    bool             isBest;
};

class AlgorithmRunner {
public:
    void run(int algoChoice, int hctTMax,
             const TSP& tsp, int nodeCount,
             int fixedStart = -1, int maxFrames = 250,
             SAParams saParams = {}, GAParams gaParams = {});

    const std::vector<AnimFrame>& frames()       const;
    const std::vector<int>&       bestRoute()    const;
    const std::vector<int>&       initialRoute() const;
    double                        bestCost()     const;
    double                        initialCost()  const;

private:
    std::vector<AnimFrame> m_frames;
    std::vector<int>       m_best;
    std::vector<int>       m_initialRoute;
    double                 m_bestCost    = 0.0;
    double                 m_initialCost = 0.0;
};
