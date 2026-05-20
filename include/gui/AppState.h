#pragma once
#include <string>

enum class AppState { MainMenu, AlgoSelect, Game, Pin, Exit };

struct TransitionData {
    int         algoChoice = 0;   // 1=HC 2=HCT 3=SA 4=GA
    int         hctTMax    = 5;
    std::string presetName;
};
