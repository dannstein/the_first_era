#include "gui/AlgorithmRunner.h"
#include "core/Randomize.h"
#include "algorithm/HC.h"
#include "algorithm/HCT.h"
#include "algorithm/SA.h"
#include "algorithm/GA.h"
#include <limits>

void AlgorithmRunner::run(int algoChoice, int hctTMax,
                          const TSP& tsp, int nodeCount,
                          int fixedStart, int maxFrames,
                          SAParams saParams, GAParams gaParams) {
    m_frames.clear();
    m_best.clear();
    m_initialRoute.clear();
    m_bestCost    = 0.0;
    m_initialCost = 0.0;

    double bestSoFar = std::numeric_limits<double>::max();

    AlgoCallback cb = [&](const std::vector<int>& route, double cost) {
        bool improved = cost < bestSoFar;
        if (improved) bestSoFar = cost;
        m_frames.push_back({route, cost, improved});
    };

    Randomize rng;

    if (algoChoice == 1) {
        auto initial = rng.randomSolution(nodeCount, fixedStart);
        double initCost = tsp.evaluate(initial);
        m_initialCost  = initCost;
        m_initialRoute = initial;
        HC hc;
        auto [sol, cost] = hc.Hill(initial, initCost, tsp, fixedStart, cb);
        m_best     = sol;
        m_bestCost = cost;

    } else if (algoChoice == 2) {
        auto initial = rng.randomSolution(nodeCount, fixedStart);
        double initCost = tsp.evaluate(initial);
        m_initialCost  = initCost;
        m_initialRoute = initial;
        HCT hct;
        auto [sol, cost] = hct.Hill(initial, initCost, tsp, hctTMax, nodeCount, fixedStart, cb);
        m_best     = sol;
        m_bestCost = cost;

    } else if (algoChoice == 3) {
        auto initial = rng.randomSolution(nodeCount, fixedStart);
        double initCost = tsp.evaluate(initial);
        m_initialCost  = initCost;
        m_initialRoute = initial;
        SA sa;
        auto [sol, cost] = sa.Annealing(initial, initCost, tsp, fixedStart, cb, saParams);
        m_best     = sol;
        m_bestCost = cost;

    } else {
        auto initial = rng.randomSolution(nodeCount, fixedStart);
        m_initialCost  = tsp.evaluate(initial);
        m_initialRoute = initial;
        GA ga;
        auto [sol, cost] = ga.run(tsp, nodeCount, fixedStart, cb, gaParams);
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

const std::vector<AnimFrame>& AlgorithmRunner::frames()       const { return m_frames; }
const std::vector<int>&       AlgorithmRunner::bestRoute()    const { return m_best; }
const std::vector<int>&       AlgorithmRunner::initialRoute() const { return m_initialRoute; }
double                        AlgorithmRunner::bestCost()     const { return m_bestCost; }
double                        AlgorithmRunner::initialCost()  const { return m_initialCost; }
