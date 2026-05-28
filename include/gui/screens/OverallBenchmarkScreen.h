#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "gui/Screen.h"
#include "gui/AssetManager.h"
#include "gui/NodePositions.h"
#include "gui/AlgorithmRunner.h"
#include "core/Graph.h"
#include "core/TSP.h"
#include "algorithm/AlgoParams.h"

class OverallBenchmarkScreen : public Screen {
public:
    OverallBenchmarkScreen(AssetManager& assets, const Graph& graph, const TSP& tsp,
                           NodePositions positions,
                           int hctTMax, int fixedStart,
                           SAParams saParams, GAParams gaParams,
                           int numRuns,
                           const std::string& projectRoot);

    void handleEvent(const sf::Event& event, AppState& next, TransitionData& data) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    struct AlgoResult {
        int                 algoChoice = 0;
        std::vector<double> initialCosts;
        std::vector<double> finalCosts;
        std::vector<int>    bestInitRoute;   // initial route of best run
        std::vector<int>    bestFinalRoute;  // best route of best run
        double              medianInitial = 0.0;
        double              medianFinal   = 0.0;
        double              gain          = 0.0;
    };

    AssetManager&   m_assets;
    const Graph&    m_graph;
    sf::Sprite      m_mapSprite;
    NodePositions   m_positions;
    const TSP&      m_tsp;

    // Run parameters
    int      m_hctTMax;
    int      m_fixedStart;
    SAParams m_saParams;
    GAParams m_gaParams;
    int      m_numRuns;
    std::string m_projectRoot;

    // Per-algo results (0=HC,1=HCT,2=SA,3=GA)
    AlgoResult      m_results[4];
    int             m_algoIdx   = 0;
    int             m_runIdx    = 0;   // 1-based after first startRun()
    AlgorithmRunner m_runner;

    // Animation state
    std::size_t     m_frameIdx  = 0;
    float           m_animTimer = 0.f;
    bool            m_animDone  = false;
    bool            m_allDone   = false;
    std::string     m_csvPath;

    // Stats scroll / hover
    int          m_statsScroll = 0;
    sf::Vector2f m_mousePos    = {-1.f, -1.f};

    // Map zoom/pan
    sf::View     m_mapView;
    float        m_currentZoom = 1.f;
    bool         m_panning     = false;
    sf::Vector2i m_panStart;

    sf::Text m_backBtn;

    void startRun();
    void finalizeAlgo();
    void writeCSV();

    void drawEdges(sf::RenderWindow& window);
    void drawPath(sf::RenderWindow& window, const std::vector<int>& route,
                  sf::Color color, float thickness);
    void drawNodes(sf::RenderWindow& window, const std::vector<int>& highlight,
                   sf::Color pathColor, bool showOrder);
    void drawProgressPanel(sf::RenderWindow& window);
    void drawFinalTable(sf::RenderWindow& window);

    static double medianOf(std::vector<double> v);
    static std::string routeToStr(const std::vector<int>& r);
    static std::string costsToStr(const std::vector<double>& v);
    static std::string configStr(int algoChoice, int hctTMax,
                                 const SAParams& sa, const GAParams& ga);
};
