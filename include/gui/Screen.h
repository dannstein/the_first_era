#pragma once
#include <SFML/Graphics.hpp>
#include "gui/AppState.h"

class Screen {
public:
    virtual ~Screen() = default;
    virtual void handleEvent(const sf::Event& event, AppState& next, TransitionData& data) = 0;
    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
};
