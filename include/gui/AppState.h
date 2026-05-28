#pragma once
#include <string>
#include "algorithm/AlgoParams.h"

enum class AppState { MainMenu, GraphSetup, AlgoSelect, ParamConfig, NodeSelect, Game, Benchmark, OverallBenchmarkSetup, OverallBenchmark, Pin, Exit };

struct TransitionData {
    int         algoChoice     = 0;   // 1=HC 2=HCT 3=SA 4=GA
    int         hctTMax        = 5;
    int         fixedStart     = 0;   // node index pinned to route[0]; -1 = free
    bool        useRandom      = false;
    int         randomCount    = 20;
    SAParams    saParams       = {};
    GAParams    gaParams       = {};
    std::string presetName;
    bool        benchmarkMode     = false;
    int         benchmarkRuns     = 20;
    bool        overallBenchmark  = false;
};
