#include "gui/screens/AlgoSelectScreen.h"
#include <SFML/Window/Keyboard.hpp>

static constexpr float WIN_W = 1600.f;
static constexpr float WIN_H = 900.f;
static constexpr float IMG_W = 1672.f;
static constexpr float IMG_H = 941.f;

AlgoSelectScreen::AlgoSelectScreen(AssetManager& assets) : m_assets(assets) {
    m_mapSprite.setTexture(m_assets.mapTexture());
    m_mapSprite.setScale(WIN_W / IMG_W, WIN_H / IMG_H);

    m_title.setFont(m_assets.font());
    m_title.setString("Select Algorithm");
    m_title.setCharacterSize(30);
    m_title.setFillColor(sf::Color(220, 200, 150));
    m_title.setStyle(sf::Text::Bold);

    const char* algoLabels[4] = {
        "[1]  Hill Climbing",
        "[2]  Hill Climbing with Tries",
        "[3]  Simulated Annealing",
        "[4]  Genetic Algorithm"
    };
    for (int i = 0; i < 4; i++) {
        m_algoOpts[i].setFont(m_assets.font());
        m_algoOpts[i].setString(algoLabels[i]);
        m_algoOpts[i].setCharacterSize(20);
        m_algoOpts[i].setFillColor(sf::Color(200, 190, 160));
    }

    m_startBtn.setFont(m_assets.font());
    m_startBtn.setString("[ Start ]");
    m_startBtn.setCharacterSize(22);
    m_startBtn.setFillColor(sf::Color(180, 220, 120));

    m_backBtn.setFont(m_assets.font());
    m_backBtn.setString("[ Back ]");
    m_backBtn.setCharacterSize(18);
    m_backBtn.setFillColor(sf::Color(180, 160, 120));
}

void AlgoSelectScreen::setSelected(int choice) {
    m_selected = choice;
}

void AlgoSelectScreen::confirm(AppState& next, TransitionData& data) const {
    if (m_selected == 0) return;
    data.algoChoice = m_selected;
    next = AppState::ParamConfig;
}

void AlgoSelectScreen::handleEvent(const sf::Event& event, AppState& next, TransitionData& data) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Num1) setSelected(1);
        if (event.key.code == sf::Keyboard::Num2) setSelected(2);
        if (event.key.code == sf::Keyboard::Num3) setSelected(3);
        if (event.key.code == sf::Keyboard::Num4) setSelected(4);
        if (event.key.code == sf::Keyboard::Return && m_selected != 0) confirm(next, data);
        if (event.key.code == sf::Keyboard::Escape) next = AppState::GraphSetup;
    }

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f click(event.mouseButton.x, event.mouseButton.y);
        for (int i = 0; i < 4; i++) {
            if (m_algoOpts[i].getGlobalBounds().contains(click))
                setSelected(i + 1);
        }
        if (m_startBtn.getGlobalBounds().contains(click) && m_selected != 0)
            confirm(next, data);
        if (m_backBtn.getGlobalBounds().contains(click))
            next = AppState::GraphSetup;
    }

    if (event.type == sf::Event::MouseMoved) {
        m_hovered = -1;
        sf::Vector2f mouse(event.mouseMove.x, event.mouseMove.y);
        for (int i = 0; i < 4; i++) {
            if (m_algoOpts[i].getGlobalBounds().contains(mouse))
                m_hovered = i + 1;
        }
    }
}

void AlgoSelectScreen::update(float /*dt*/) {}

void AlgoSelectScreen::draw(sf::RenderWindow& window) {
    window.draw(m_mapSprite);

    sf::RectangleShape overlay(sf::Vector2f(WIN_W, WIN_H));
    overlay.setFillColor(sf::Color(0, 0, 0, 120));
    window.draw(overlay);

    constexpr float PW = 460.f;
    constexpr float PH = 280.f;
    float px = (WIN_W - PW) / 2.f;
    float py = (WIN_H - PH) / 2.f;

    sf::RectangleShape panel(sf::Vector2f(PW, PH));
    panel.setPosition(px, py);
    panel.setFillColor(sf::Color(10, 8, 6, 220));
    panel.setOutlineColor(sf::Color(150, 120, 60));
    panel.setOutlineThickness(2.f);
    window.draw(panel);

    auto tb = m_title.getLocalBounds();
    m_title.setOrigin(tb.left + tb.width / 2.f, tb.top);
    m_title.setPosition(px + PW / 2.f, py + 16.f);
    window.draw(m_title);

    for (int i = 0; i < 4; i++) {
        bool sel   = (m_selected == i + 1);
        bool hover = (m_hovered  == i + 1);
        sf::Color col = sel   ? sf::Color(255, 240,  80) :
                        hover ? sf::Color(255, 240, 180) :
                                sf::Color(200, 190, 160);
        m_algoOpts[i].setStyle(sel ? sf::Text::Bold : sf::Text::Regular);
        m_algoOpts[i].setFillColor(col);
        m_algoOpts[i].setPosition(px + 30.f, py + 60.f + i * 36.f);
        window.draw(m_algoOpts[i]);
    }

    float nextY = py + 60.f + 4 * 36.f + 10.f;

    m_startBtn.setPosition(px + PW - 130.f, nextY);
    window.draw(m_startBtn);

    m_backBtn.setPosition(px + 20.f, nextY + 4.f);
    window.draw(m_backBtn);
}
