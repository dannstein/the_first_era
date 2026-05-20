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
    Graph                   m_graph{1};   // resized in init()
    TSP                     m_tsp{1};     // resized in init()
    NodePositions           m_positions;
    AppState                m_state  = AppState::MainMenu;
    TransitionData          m_data;
    std::unique_ptr<Screen> m_screen;

    void init();
    void switchTo(AppState next, const TransitionData& data = {});
};
