#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "gui/Screen.h"
#include "gui/AssetManager.h"
#include "gui/NodePositions.h"
#include "core/Graph.h"

class PinScreen : public Screen {
public:
    PinScreen(AssetManager& assets, const Graph& graph,
              NodePositions positions, const std::string& configDir);

    void handleEvent(const sf::Event& event, AppState& next, TransitionData& data) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    const NodePositions& positions() const;

private:
    static constexpr float SIDEBAR_W  = 230.f;
    static constexpr float ITEM_H     = 17.f;
    static constexpr float BOT_AREA   = 95.f;
    static constexpr float MAP_IMG_W  = 1672.f;
    static constexpr float MAP_IMG_H  = 941.f;
    static constexpr float NODE_R     = 8.f;   // world-space radius

    struct RegionDef { const char* name; int first, last; sf::Color color; };
    static const RegionDef REGIONS[6];

    static int rowToNode(int row);
    static constexpr int TOTAL_ROWS = 50 + 6;

    AssetManager& m_assets;
    const Graph&  m_graph;
    NodePositions m_positions;
    std::string   m_configDir;

    sf::Sprite    m_mapSprite;

    // View for zoom/pan of the map area
    sf::View      m_mapView;
    float         m_currentZoom = 1.f;
    bool          m_panning     = false;
    sf::Vector2i  m_panStart;

    int           m_selected    = -1;
    int           m_sideScroll  = 0;
    bool          m_editingName = false;
    std::string   m_presetName;

    sf::Text           m_saveBtn;
    sf::Text           m_backBtn;
    sf::Text           m_presetLabel;
    sf::Text           m_presetInput;

    // Explicit hit rects updated by drawSidebar() — avoids relying on
    // sf::Text::getGlobalBounds() which returns tight glyph-only bounds.
    sf::FloatRect      m_saveBtnRect;
    sf::FloatRect      m_backBtnRect;
    sf::FloatRect      m_presetInputRect;

    // Set by draw() so handleEvent can convert pixel→world coordinates.
    sf::RenderWindow*  m_window = nullptr;

    void drawSidebar(sf::RenderWindow& window);
    void drawMapArea(sf::RenderWindow& window);
    int  nodeNear(sf::Vector2i screenPos, float worldRadius = 14.f) const;
};
