#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "gui/Screen.h"
#include "gui/AssetManager.h"
#include "gui/NodePositions.h"
#include "gui/AlgorithmRunner.h"
#include "core/Graph.h"
#include "core/TSP.h"
#include "algorithm/AlgoParams.h"

class BenchmarkScreen : public Screen {
public:
    BenchmarkScreen(AssetManager& assets, const Graph& graph, const TSP& tsp,
                    NodePositions positions,
                    int algoChoice, int hctTMax, int fixedStart,
                    SAParams saParams, GAParams gaParams,
                    int numRuns);

    void handleEvent(const sf::Event& event, AppState& next, TransitionData& data) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    AssetManager&   m_assets;
    const Graph&    m_graph;
    sf::Sprite      m_mapSprite;
    NodePositions   m_positions;

    // Run parameters
    int      m_algoChoice;
    int      m_hctTMax;
    int      m_fixedStart;
    SAParams m_saParams;
    GAParams m_gaParams;
    int      m_numRuns;
    const TSP& m_tsp;

    // Per-run results
    std::vector<double> m_initialCosts;
    std::vector<double> m_finalCosts;

    // Current run state
    AlgorithmRunner m_runner;
    int             m_currentRun  = 0;   // 1-based after first startRun()
    std::size_t     m_frameIdx    = 0;
    float           m_animTimer   = 0.f;
    bool            m_animDone    = false;
    bool            m_allDone     = false;

    // Final median results (set when allDone)
    double m_medianInitial = 0.0;
    double m_medianFinal   = 0.0;
    double m_medianGain    = 0.0;

    // Stats scroll (for final panel)
    int          m_statsScroll = 0;
    sf::Vector2f m_mousePos    = {-1.f, -1.f};
    std::vector<std::vector<double>> m_distMatrix;

    // Cached APSP distances for per-edge tooltip
    sf::Text m_backBtn;
    sf::Text m_zoomHint;

    // View for map zoom/pan
    sf::View     m_mapView;
    float        m_currentZoom = 1.f;
    bool         m_panning     = false;
    sf::Vector2i m_panStart;

    void startRun();
    void computeMedians();

    void drawEdges(sf::RenderWindow& window);
    void drawPath(sf::RenderWindow& window, const std::vector<int>& route,
                  sf::Color color, float thickness);
    void drawNodes(sf::RenderWindow& window, const std::vector<int>& highlight,
                   sf::Color pathColor, bool showOrder);
    void drawProgress(sf::RenderWindow& window);
    void drawFinalStats(sf::RenderWindow& window);
};
