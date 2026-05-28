#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "gui/Screen.h"
#include "gui/AssetManager.h"

class GraphSetupScreen : public Screen {
public:
    explicit GraphSetupScreen(AssetManager& assets);

    void handleEvent(const sf::Event& event, AppState& next, TransitionData& data) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    static constexpr float WIN_W = 1600.f;
    static constexpr float WIN_H = 900.f;
    static constexpr float IMG_W = 1672.f;
    static constexpr float IMG_H = 941.f;

    AssetManager& m_assets;
    sf::Sprite    m_mapSprite;
    sf::Text      m_title;

    int           m_selected  = 1;   // 1 = Default, 2 = Random
    std::string   m_countStr;        // typed node count for random mode
    int           m_hovered   = -1;

    // Hit rects set during draw()
    sf::FloatRect m_defaultCardRect;
    sf::FloatRect m_randomCardRect;
    sf::FloatRect m_countInputRect;
    sf::FloatRect m_continueBtnRect;
    sf::FloatRect m_backBtnRect;

    int  parsedCount() const;        // returns clamped int from m_countStr
    void confirm(AppState& next, TransitionData& data) const;
};
