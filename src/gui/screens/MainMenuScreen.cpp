#include "gui/screens/MainMenuScreen.h"
#include <SFML/Window/Keyboard.hpp>

static constexpr float WIN_W = 1600.f;
static constexpr float WIN_H = 900.f;
static constexpr float IMG_W = 1672.f;
static constexpr float IMG_H = 941.f;

MainMenuScreen::MainMenuScreen(AssetManager& assets) : m_assets(assets) {
    // Scale map to fill window
    m_mapSprite.setTexture(m_assets.mapTexture());
    m_mapSprite.setScale(WIN_W / IMG_W, WIN_H / IMG_H);

    // Title
    m_title.setFont(m_assets.font());
    m_title.setString("The First Era");
    m_title.setCharacterSize(34);
    m_title.setFillColor(sf::Color(220, 200, 150));
    m_title.setStyle(sf::Text::Bold);

    const char* labels[3] = {
        "[1]  Enter Game",
        "[2]  Pin Landmarks",
        "[3]  Exit"
    };
    for (int i = 0; i < 3; i++) {
        m_opts[i].setFont(m_assets.font());
        m_opts[i].setString(labels[i]);
        m_opts[i].setCharacterSize(22);
        m_opts[i].setFillColor(sf::Color(200, 190, 160));
    }
}

void MainMenuScreen::select(int choice, AppState& next, TransitionData& data) {
    if      (choice == 1) { next = AppState::AlgoSelect; }
    else if (choice == 2) { next = AppState::Pin; }
    else if (choice == 3) { next = AppState::Exit; }
    (void)data;
}

void MainMenuScreen::handleEvent(const sf::Event& event, AppState& next, TransitionData& data) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Num1) select(1, next, data);
        if (event.key.code == sf::Keyboard::Num2) select(2, next, data);
        if (event.key.code == sf::Keyboard::Num3) select(3, next, data);
    }
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f click(event.mouseButton.x, event.mouseButton.y);
        for (int i = 0; i < 3; i++) {
            if (m_opts[i].getGlobalBounds().contains(click))
                select(i + 1, next, data);
        }
    }
    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2f mouse(event.mouseMove.x, event.mouseMove.y);
        for (int i = 0; i < 3; i++) {
            bool hover = m_opts[i].getGlobalBounds().contains(mouse);
            m_opts[i].setFillColor(hover ? sf::Color(255, 240, 180) : sf::Color(200, 190, 160));
        }
    }
}

void MainMenuScreen::update(float /*dt*/) {}

void MainMenuScreen::draw(sf::RenderWindow& window) {
    window.draw(m_mapSprite);

    // Dark overlay
    sf::RectangleShape overlay(sf::Vector2f(WIN_W, WIN_H));
    overlay.setFillColor(sf::Color(0, 0, 0, 120));
    window.draw(overlay);

    // Menu panel
    constexpr float PW = 400.f, PH = 220.f;
    float px = (WIN_W - PW) / 2.f;
    float py = (WIN_H - PH) / 2.f;
    sf::RectangleShape panel(sf::Vector2f(PW, PH));
    panel.setPosition(px, py);
    panel.setFillColor(sf::Color(10, 8, 6, 220));
    panel.setOutlineColor(sf::Color(150, 120, 60));
    panel.setOutlineThickness(2.f);
    window.draw(panel);

    // Title
    auto tb = m_title.getLocalBounds();
    m_title.setOrigin(tb.left + tb.width / 2.f, tb.top);
    m_title.setPosition(px + PW / 2.f, py + 20.f);
    window.draw(m_title);

    // Options
    for (int i = 0; i < 3; i++) {
        m_opts[i].setPosition(px + 40.f, py + 80.f + i * 40.f);
        window.draw(m_opts[i]);
    }
}
