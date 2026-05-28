#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include "core/Graph.h"
#include "core/TSP.h"
#include "gui/AppState.h"
#include "gui/AssetManager.h"
#include "gui/NodePositions.h"
#include "gui/Screen.h"

class Application {
public:
    void run();

private:
    sf::RenderWindow        m_window;
    AssetManager            m_assets;
    Graph                   m_graph{1};       // default 50-node WorldMap
    TSP                     m_tsp{1};
    NodePositions           m_positions;
    Graph                   m_randomGraph{1}; // generated on-demand for random mode
    TSP                     m_randomTsp{1};
    NodePositions           m_randomPositions;
    AppState                m_state  = AppState::MainMenu;
    TransitionData          m_data;
    std::unique_ptr<Screen> m_screen;

    void init();
    void switchTo(AppState next, const TransitionData& data = {});
};
