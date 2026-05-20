#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "gui/Screen.h"
#include "gui/AssetManager.h"

class AlgoSelectScreen : public Screen {
public:
    explicit AlgoSelectScreen(AssetManager& assets);

    void handleEvent(const sf::Event& event, AppState& next, TransitionData& data) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    AssetManager& m_assets;
    sf::Sprite    m_mapSprite;
    sf::Text      m_title;
    sf::Text      m_algoOpts[4];
    sf::Text      m_hctLabel;
    sf::Text      m_hctInput;
    sf::Text      m_startBtn;
    sf::Text      m_backBtn;

    int           m_selected   = 0;   // 0 = none chosen yet
    int           m_hovered    = -1;  // tracked separately to avoid color bleed
    bool          m_enteringT  = false;
    std::string   m_tMaxStr;

    void setSelected(int choice);     // sets m_selected and auto-activates t_max input

    void confirm(AppState& next, TransitionData& data);
};
