#pragma once
#include <SFML/Graphics.hpp>
#include "gui/Screen.h"
#include "gui/AssetManager.h"
#include "gui/NodePositions.h"
#include "gui/AlgorithmRunner.h"
#include "core/Graph.h"
#include "core/TSP.h"

class GameScreen : public Screen {
public:
    GameScreen(AssetManager& assets, const Graph& graph, const TSP& tsp,
               NodePositions positions,
               int algoChoice, int hctTMax);

    void handleEvent(const sf::Event& event, AppState& next, TransitionData& data) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    AssetManager&   m_assets;
    const Graph&    m_graph;
    sf::Sprite      m_mapSprite;
    NodePositions   m_positions;
    AlgorithmRunner m_runner;

    // View for zoom/pan of the map area
    sf::View        m_mapView;
    float           m_currentZoom = 1.f;
    bool            m_panning     = false;
    sf::Vector2i    m_panStart;

    // Animation state
    std::size_t     m_frameIdx    = 0;
    float           m_animTimer   = 0.f;
    bool            m_animDone    = false;

    // Stats overlay scroll
    int             m_statsScroll = 0;

    sf::Text        m_backBtn;
    sf::Text        m_zoomHint;

    void drawEdges(sf::RenderWindow& window);
    void drawNodes(sf::RenderWindow& window, const std::vector<int>& highlight,
                   sf::Color pathColor, bool showOrder);
    void drawPath(sf::RenderWindow& window, const std::vector<int>& route,
                  sf::Color color, float thickness);
    void drawStats(sf::RenderWindow& window);
};
