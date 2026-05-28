#pragma once

// Runtime-configurable parameters for SA and GA.
// Defaults match the static constexpr values in each algorithm header.

struct SAParams {
    double TI = 1000.0;   // initial temperature
    double TF = 0.001;    // final temperature
    double FR = 0.995;    // cooling rate per iteration
};

struct GAParams {
    int    popSize        = 200;
    int    generations    = 500;
    double mutationRate   = 0.08;  // probability [0,1]
    int    tournamentSize = 3;
    double gi             = 0.1;   // elite fraction [0,1]
    double br             = 0.8;   // breed fraction [0,1]
    int    stagnation     = 75;
};
