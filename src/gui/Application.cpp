#include "gui/Application.h"
#include "core/WorldMap.h"
#include "gui/screens/MainMenuScreen.h"
#include "gui/screens/AlgoSelectScreen.h"
#include "gui/screens/GameScreen.h"
#include "gui/screens/PinScreen.h"
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
        case AppState::AlgoSelect:
            m_screen = std::make_unique<AlgoSelectScreen>(m_assets);
            break;
        case AppState::Game: {
            // Determine positions to use: try the named preset first
            NodePositions pos;
            std::string path = NodePositions::presetPath(CFG_DIR, data.presetName);
            if (!pos.loadFromFile(path)) {
                // Fall back to default WorldMap preset
                pos.loadFromFile(NodePositions::presetPath(CFG_DIR, "WorldMap"));
            }
            m_screen = std::make_unique<GameScreen>(
                m_assets, m_graph, m_tsp,
                std::move(pos),
                data.algoChoice, data.hctTMax);
            break;
        }
        case AppState::Pin:
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

        // Handle preset selection before launching game
        if (nextState == AppState::Game && nextData.presetName.empty()) {
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
