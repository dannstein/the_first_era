#include "gui/AlgorithmRunner.h"
#include "core/Randomize.h"
#include "algorithm/HC.h"
#include "algorithm/HCT.h"
#include "algorithm/SA.h"
#include "algorithm/GA.h"
#include <limits>

void AlgorithmRunner::run(int algoChoice, int hctTMax,
                          const TSP& tsp, int nodeCount,
                          int maxFrames) {
    m_frames.clear();
    m_best.clear();
    m_bestCost = 0.0;

    double bestSoFar = std::numeric_limits<double>::max();

    AlgoCallback cb = [&](const std::vector<int>& route, double cost) {
        bool improved = cost < bestSoFar;
        if (improved) bestSoFar = cost;
        m_frames.push_back({route, cost, improved});
    };

    Randomize rng;

    if (algoChoice == 1) {
        auto initial = rng.randomSolution(nodeCount);
        double initCost = tsp.evaluate(initial);
        HC hc;
        auto [sol, cost] = hc.Hill(initial, initCost, tsp, cb);
        m_best     = sol;
        m_bestCost = cost;

    } else if (algoChoice == 2) {
        auto initial = rng.randomSolution(nodeCount);
        double initCost = tsp.evaluate(initial);
        HCT hct;
        auto [sol, cost] = hct.Hill(initial, initCost, tsp, hctTMax, cb);
        m_best     = sol;
        m_bestCost = cost;

    } else if (algoChoice == 3) {
        auto initial = rng.randomSolution(nodeCount);
        double initCost = tsp.evaluate(initial);
        SA sa;
        auto [sol, cost] = sa.Annealing(initial, initCost, tsp, cb);
        m_best     = sol;
        m_bestCost = cost;

    } else {
        GA ga;
        auto [sol, cost] = ga.run(tsp, nodeCount, cb);
        m_best     = sol;
        m_bestCost = cost;
    }

    // Sub-sample to maxFrames if needed
    if ((int)m_frames.size() > maxFrames) {
        std::vector<AnimFrame> sampled;
        sampled.reserve(maxFrames);
        double step = (double)(m_frames.size() - 1) / (maxFrames - 1);
        for (int i = 0; i < maxFrames; i++) {
            int idx = (int)(i * step + 0.5);
            sampled.push_back(std::move(m_frames[idx]));
        }
        m_frames = std::move(sampled);
    }

    // Ensure the final best route is the last frame
    if (m_frames.empty() || m_frames.back().route != m_best) {
        m_frames.push_back({m_best, m_bestCost, true});
    }
}

const std::vector<AnimFrame>& AlgorithmRunner::frames()    const { return m_frames; }
const std::vector<int>&       AlgorithmRunner::bestRoute() const { return m_best; }
double                        AlgorithmRunner::bestCost()  const { return m_bestCost; }
