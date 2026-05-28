#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "gui/Screen.h"
#include "gui/AssetManager.h"
#include "core/Graph.h"

class NodeSelectScreen : public Screen {
public:
    NodeSelectScreen(AssetManager& assets, const Graph& graph);

    void handleEvent(const sf::Event& event, AppState& next, TransitionData& data) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    static constexpr float WIN_W    = 1600.f;
    static constexpr float WIN_H    = 900.f;
    static constexpr float IMG_W    = 1672.f;
    static constexpr float IMG_H    = 941.f;
    static constexpr float PANEL_W  = 540.f;
    static constexpr float PANEL_H  = 590.f;
    static constexpr float LIST_H   = 350.f;
    static constexpr float ROW_H    = 22.f;

    struct ListItem {
        bool        isHeader;
        int         nodeId;      // -1 for header rows
        std::string label;
        sf::Color   color;
    };

    AssetManager&         m_assets;
    sf::Sprite            m_mapSprite;

    std::vector<ListItem> m_items;
    int                   m_selectedNode = 0;
    int                   m_scroll       = 0;   // first visible item index

    sf::Text m_title;
    sf::Text m_subtitle;

    // Hit rects updated in draw()
    sf::FloatRect m_listRect;
    sf::FloatRect m_defaultBtnRect;
    sf::FloatRect m_startBtnRect;
    sf::FloatRect m_backBtnRect;

    void buildItemList(const Graph& graph);
    void clampScroll();
    void scrollToSelected();
};
