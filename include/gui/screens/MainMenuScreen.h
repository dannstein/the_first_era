#pragma once
#include <SFML/Graphics.hpp>
#include "gui/Screen.h"
#include "gui/AssetManager.h"

class MainMenuScreen : public Screen {
public:
    explicit MainMenuScreen(AssetManager& assets);

    void handleEvent(const sf::Event& event, AppState& next, TransitionData& data) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    AssetManager& m_assets;
    sf::Sprite    m_mapSprite;
    sf::Text      m_title;
    sf::Text      m_opts[4];

    void select(int choice, AppState& next, TransitionData& data);
};
