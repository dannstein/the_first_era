#include "gui/Application.h"
#include "core/WorldMap.h"
#include "gui/screens/MainMenuScreen.h"
#include "gui/screens/GraphSetupScreen.h"
#include "gui/screens/AlgoSelectScreen.h"
#include "gui/screens/ParamConfigScreen.h"
#include "gui/screens/NodeSelectScreen.h"
#include "gui/screens/GameScreen.h"
#include "gui/screens/BenchmarkScreen.h"
#include "gui/screens/OverallBenchmarkSetupScreen.h"
#include "gui/screens/OverallBenchmarkScreen.h"
#include "gui/screens/PinScreen.h"
#include "core/RandomGraph.h"
#include <iostream>
#include <string>

#ifndef PROJECT_ROOT
#define PROJECT_ROOT "."
#endif

static const std::string ROOT     = PROJECT_ROOT;
static const std::string CFG_DIR  = ROOT + "/config";

void Application::init() {
    m_window.create(sf::VideoMode(1600, 900), "The First Era",
                    sf::Style::Titlebar | sf::Style::Close);
    m_window.setFramerateLimit(60);

    if (!m_assets.load(ROOT)) {
        std::cerr << "Failed to load assets. Exiting.\n";
        m_window.close();
        return;
    }

    m_graph = buildWorldMap();
    m_tsp   = TSP(50);
    m_tsp.addGraph(m_graph);

    m_positions.loadFromFile(NodePositions::presetPath(CFG_DIR, "WorldMap"));

    switchTo(AppState::MainMenu);
}

void Application::switchTo(AppState next, const TransitionData& data) {
    m_state = next;
    m_data  = data;

    switch (next) {
        case AppState::MainMenu:
            m_screen = std::make_unique<MainMenuScreen>(m_assets);
            break;
        case AppState::GraphSetup:
            m_screen = std::make_unique<GraphSetupScreen>(m_assets);
            break;
        case AppState::AlgoSelect:
            m_screen = std::make_unique<AlgoSelectScreen>(m_assets);
            break;
        case AppState::ParamConfig:
            m_screen = std::make_unique<ParamConfigScreen>(m_assets, data.algoChoice);
            break;
        case AppState::NodeSelect:
            m_screen = std::make_unique<NodeSelectScreen>(m_assets, m_graph);
            break;
        case AppState::Game: {
            if (data.useRandom) {
                // Generate a fresh random graph each run
                m_randomPositions = NodePositions(data.randomCount);
                m_randomGraph    = generateRandomGraph(data.randomCount, m_randomPositions);
                m_randomTsp      = TSP(data.randomCount);
                m_randomTsp.addGraph(m_randomGraph);
                m_screen = std::make_unique<GameScreen>(
                    m_assets, m_randomGraph, m_randomTsp,
                    m_randomPositions,
                    data.algoChoice, data.hctTMax, 0,
                    data.saParams, data.gaParams);
            } else {
                // Determine positions to use: try the named preset first
                NodePositions pos;
                std::string path = NodePositions::presetPath(CFG_DIR, data.presetName);
                if (!pos.loadFromFile(path)) {
                    pos.loadFromFile(NodePositions::presetPath(CFG_DIR, "WorldMap"));
                }
                m_screen = std::make_unique<GameScreen>(
                    m_assets, m_graph, m_tsp,
                    std::move(pos),
                    data.algoChoice, data.hctTMax, data.fixedStart,
                    data.saParams, data.gaParams);
            }
            break;
        }
        case AppState::Benchmark: {
            if (data.useRandom) {
                m_randomPositions = NodePositions(data.randomCount);
                m_randomGraph     = generateRandomGraph(data.randomCount, m_randomPositions);
                m_randomTsp       = TSP(data.randomCount);
                m_randomTsp.addGraph(m_randomGraph);
                m_screen = std::make_unique<BenchmarkScreen>(
                    m_assets, m_randomGraph, m_randomTsp,
                    m_randomPositions,
                    data.algoChoice, data.hctTMax, 0,
                    data.saParams, data.gaParams,
                    data.benchmarkRuns);
            } else {
                NodePositions pos;
                std::string path = NodePositions::presetPath(CFG_DIR, data.presetName);
                if (!pos.loadFromFile(path))
                    pos.loadFromFile(NodePositions::presetPath(CFG_DIR, "WorldMap"));
                m_screen = std::make_unique<BenchmarkScreen>(
                    m_assets, m_graph, m_tsp,
                    std::move(pos),
                    data.algoChoice, data.hctTMax, data.fixedStart,
                    data.saParams, data.gaParams,
                    data.benchmarkRuns);
            }
            break;
        }
        case AppState::OverallBenchmarkSetup:
            m_screen = std::make_unique<OverallBenchmarkSetupScreen>(m_assets);
            break;
        case AppState::OverallBenchmark: {
            if (data.useRandom) {
                m_randomPositions = NodePositions(data.randomCount);
                m_randomGraph     = generateRandomGraph(data.randomCount, m_randomPositions);
                m_randomTsp       = TSP(data.randomCount);
                m_randomTsp.addGraph(m_randomGraph);
                m_screen = std::make_unique<OverallBenchmarkScreen>(
                    m_assets, m_randomGraph, m_randomTsp,
                    m_randomPositions,
                    data.hctTMax, 0,
                    data.saParams, data.gaParams,
                    data.benchmarkRuns, ROOT);
            } else {
                NodePositions pos;
                std::string path = NodePositions::presetPath(CFG_DIR, data.presetName);
                if (!pos.loadFromFile(path))
                    pos.loadFromFile(NodePositions::presetPath(CFG_DIR, "WorldMap"));
                m_screen = std::make_unique<OverallBenchmarkScreen>(
                    m_assets, m_graph, m_tsp,
                    std::move(pos),
                    data.hctTMax, data.fixedStart,
                    data.saParams, data.gaParams,
                    data.benchmarkRuns, ROOT);
            }
            break;
        }
        case AppState::Pin:
            // Always reload from disk so previously saved changes are reflected.
            m_positions.loadFromFile(NodePositions::presetPath(CFG_DIR, "WorldMap"));
            m_screen = std::make_unique<PinScreen>(
                m_assets, m_graph, m_positions, CFG_DIR);
            break;
        case AppState::Exit:
            m_window.close();
            break;
    }
}

void Application::run() {
    init();
    if (!m_window.isOpen()) return;

    sf::Clock clock;
    while (m_window.isOpen()) {
        float dt = clock.restart().asSeconds();

        AppState nextState = m_state;
        TransitionData nextData = m_data;

        sf::Event event;
        while (m_window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                m_window.close();
                return;
            }
            if (m_screen)
                m_screen->handleEvent(event, nextState, nextData);
        }

        // Handle preset selection before launching game/benchmark (default graph only)
        bool launchingMap = (nextState == AppState::Game ||
                             nextState == AppState::Benchmark ||
                             nextState == AppState::OverallBenchmark);
        if (launchingMap && !nextData.useRandom && nextData.presetName.empty()) {
            auto presets = NodePositions::listPresets(CFG_DIR);
            if (presets.size() > 1) {
                // For simplicity: auto-pick "WorldMap" if it exists, else first
                for (auto& p : presets) {
                    if (p == "WorldMap") { nextData.presetName = p; break; }
                }
                if (nextData.presetName.empty())
                    nextData.presetName = presets[0];
            }
        }

        if (nextState != m_state)
            switchTo(nextState, nextData);

        if (m_screen) m_screen->update(dt);

        m_window.clear(sf::Color(20, 16, 12));
        if (m_screen) m_screen->draw(m_window);
        m_window.display();
    }
}
