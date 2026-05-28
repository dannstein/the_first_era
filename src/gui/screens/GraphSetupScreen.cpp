#include "gui/screens/GraphSetupScreen.h"
#include <SFML/Window/Keyboard.hpp>
#include <algorithm>

static constexpr float WIN_W = 1600.f;
static constexpr float WIN_H = 900.f;
static constexpr float IMG_W = 1672.f;
static constexpr float IMG_H = 941.f;

static constexpr int COUNT_MIN = 5;
static constexpr int COUNT_MAX = 100;
static constexpr int COUNT_DEF = 20;

GraphSetupScreen::GraphSetupScreen(AssetManager& assets) : m_assets(assets) {
    m_mapSprite.setTexture(m_assets.mapTexture());
    m_mapSprite.setScale(WIN_W / IMG_W, WIN_H / IMG_H);

    m_title.setFont(m_assets.font());
    m_title.setString("Graph Setup");
    m_title.setCharacterSize(30);
    m_title.setFillColor(sf::Color(220, 200, 150));
    m_title.setStyle(sf::Text::Bold);

    m_countStr = std::to_string(COUNT_DEF);
}

int GraphSetupScreen::parsedCount() const {
    if (m_countStr.empty()) return COUNT_DEF;
    int v = std::stoi(m_countStr);
    return std::max(COUNT_MIN, std::min(COUNT_MAX, v));
}

void GraphSetupScreen::confirm(AppState& next, TransitionData& data) const {
    data.useRandom   = (m_selected == 2);
    data.randomCount = (m_selected == 2) ? parsedCount() : 50;
    next = AppState::AlgoSelect;
}

void GraphSetupScreen::handleEvent(const sf::Event& event, AppState& next, TransitionData& data) {
    if (event.type == sf::Event::KeyPressed) {
        if (m_selected == 2) {
            // Typing mode: digits go to TextEntered, only handle control keys here
            if (event.key.code == sf::Keyboard::BackSpace && !m_countStr.empty())
                m_countStr.pop_back();
            else if (event.key.code == sf::Keyboard::Return)
                confirm(next, data);
            else if (event.key.code == sf::Keyboard::Escape)
                next = AppState::MainMenu;
            // Do NOT handle Num1/Num2 here — they would steal digits from the text field
        } else {
            if (event.key.code == sf::Keyboard::Num1) m_selected = 1;
            if (event.key.code == sf::Keyboard::Num2) { m_selected = 2; return; }
            if (event.key.code == sf::Keyboard::Return) confirm(next, data);
            if (event.key.code == sf::Keyboard::Escape) next = AppState::MainMenu;
        }
    }

    // Digit input for the count field when random is selected
    if (event.type == sf::Event::TextEntered && m_selected == 2) {
        char c = static_cast<char>(event.text.unicode);
        if (c >= '0' && c <= '9' && m_countStr.size() < 3) {
            // Don't allow leading zeros
            if (!(m_countStr.empty() && c == '0'))
                m_countStr += c;
        }
    }

    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f click((float)event.mouseButton.x, (float)event.mouseButton.y);

        if (m_defaultCardRect.contains(click)) { m_selected = 1; return; }
        if (m_randomCardRect.contains(click))  { m_selected = 2; return; }
        if (m_continueBtnRect.contains(click)) { confirm(next, data); return; }
        if (m_backBtnRect.contains(click))     { next = AppState::MainMenu; return; }
    }

    if (event.type == sf::Event::MouseMoved) {
        m_hovered = -1;
        sf::Vector2f mouse((float)event.mouseMove.x, (float)event.mouseMove.y);
        if (m_defaultCardRect.contains(mouse)) m_hovered = 1;
        if (m_randomCardRect.contains(mouse))  m_hovered = 2;
    }
}

void GraphSetupScreen::update(float /*dt*/) {}

void GraphSetupScreen::draw(sf::RenderWindow& window) {
    window.draw(m_mapSprite);

    sf::RectangleShape overlay(sf::Vector2f(WIN_W, WIN_H));
    overlay.setFillColor(sf::Color(0, 0, 0, 125));
    window.draw(overlay);

    // Main panel
    constexpr float PW = 600.f;
    const float PH = (m_selected == 2) ? 330.f : 290.f;
    float px = (WIN_W - PW) / 2.f;
    float py = (WIN_H - PH) / 2.f;

    sf::RectangleShape panel(sf::Vector2f(PW, PH));
    panel.setPosition(px, py);
    panel.setFillColor(sf::Color(10, 8, 6, 225));
    panel.setOutlineColor(sf::Color(150, 120, 60));
    panel.setOutlineThickness(2.f);
    window.draw(panel);

    // Title
    auto tb = m_title.getLocalBounds();
    m_title.setOrigin(tb.left + tb.width / 2.f, tb.top);
    m_title.setPosition(px + PW / 2.f, py + 14.f);
    window.draw(m_title);

    // Subtitle
    sf::Text sub;
    sub.setFont(m_assets.font());
    sub.setCharacterSize(14);
    sub.setFillColor(sf::Color(160, 150, 120));
    sub.setString("Choose the graph you want the algorithms to optimize.");
    auto sb = sub.getLocalBounds();
    sub.setOrigin(sb.left + sb.width / 2.f, sb.top);
    sub.setPosition(px + PW / 2.f, py + 50.f);
    window.draw(sub);

    // ── Option cards ─────────────────────────────────────────────────────────
    constexpr float CARD_W = 260.f;
    constexpr float CARD_H = 110.f;
    float cardY = py + 76.f;
    float card1X = px + 20.f;
    float card2X = px + PW - CARD_W - 20.f;

    auto drawCard = [&](float cx, float cy, int choice,
                         const std::string& title, const std::string& line1,
                         const std::string& line2, sf::FloatRect& outRect) {
        bool sel   = (m_selected == choice);
        bool hover = (m_hovered  == choice);

        sf::Color border = sel   ? sf::Color(220, 185,  60, 255) :
                           hover ? sf::Color(160, 140,  80, 200) :
                                   sf::Color( 80,  65,  35, 180);
        sf::Color bg     = sel   ? sf::Color(28, 22, 8, 220) :
                                   sf::Color(16, 12, 6, 200);

        sf::RectangleShape card(sf::Vector2f(CARD_W, CARD_H));
        card.setPosition(cx, cy);
        card.setFillColor(bg);
        card.setOutlineColor(border);
        card.setOutlineThickness(sel ? 2.f : 1.f);
        window.draw(card);

        sf::Text hdr;
        hdr.setFont(m_assets.font());
        hdr.setCharacterSize(16);
        hdr.setStyle(sel ? sf::Text::Bold : sf::Text::Regular);
        hdr.setFillColor(sel ? sf::Color(255, 235, 100) : sf::Color(200, 185, 145));
        hdr.setString(title);
        hdr.setPosition(cx + 12.f, cy + 10.f);
        window.draw(hdr);

        sf::Text l1;
        l1.setFont(m_assets.font());
        l1.setCharacterSize(13);
        l1.setFillColor(sf::Color(160, 150, 120));
        l1.setString(line1);
        l1.setPosition(cx + 12.f, cy + 38.f);
        window.draw(l1);

        sf::Text l2;
        l2.setFont(m_assets.font());
        l2.setCharacterSize(13);
        l2.setFillColor(sf::Color(160, 150, 120));
        l2.setString(line2);
        l2.setPosition(cx + 12.f, cy + 58.f);
        window.draw(l2);

        // Selection indicator
        if (sel) {
            sf::Text tick;
            tick.setFont(m_assets.font());
            tick.setCharacterSize(13);
            tick.setFillColor(sf::Color(180, 220, 100));
            tick.setString("[selected]");
            tick.setPosition(cx + 12.f, cy + CARD_H - 22.f);
            window.draw(tick);
        }

        outRect = sf::FloatRect(cx, cy, CARD_W, CARD_H);
    };

    drawCard(card1X, cardY, 1,
             "[1]  Default WorldMap",
             "50 nodes, 6 themed regions",
             "Story layout, named landmarks",
             m_defaultCardRect);

    drawCard(card2X, cardY, 2,
             "[2]  Random Graph",
             "N nodes, map-wide coverage",
             "Random positions & edge weights",
             m_randomCardRect);

    float nextY = cardY + CARD_H + 14.f;

    // ── Node count input (random only) ───────────────────────────────────────
    if (m_selected == 2) {
        sf::Text lbl;
        lbl.setFont(m_assets.font());
        lbl.setCharacterSize(15);
        lbl.setFillColor(sf::Color(200, 185, 145));
        lbl.setString("Number of nodes  (" + std::to_string(COUNT_MIN) + "-" +
                       std::to_string(COUNT_MAX) + "):");
        lbl.setPosition(px + 20.f, nextY + 4.f);
        window.draw(lbl);

        // Input box
        float boxX = px + 270.f;
        float boxY = nextY;
        float boxW = 80.f;
        float boxH = 26.f;
        sf::RectangleShape box(sf::Vector2f(boxW, boxH));
        box.setPosition(boxX, boxY);
        box.setFillColor(sf::Color(22, 18, 10, 220));
        box.setOutlineColor(sf::Color(140, 115, 55));
        box.setOutlineThickness(1.5f);
        window.draw(box);

        std::string display = m_countStr.empty() ? "_" : m_countStr + "_";
        sf::Text inp;
        inp.setFont(m_assets.font());
        inp.setCharacterSize(15);
        inp.setFillColor(sf::Color(255, 240, 160));
        inp.setString(display);
        inp.setPosition(boxX + 6.f, boxY + 4.f);
        window.draw(inp);

        m_countInputRect = sf::FloatRect(boxX, boxY, boxW, boxH);
        nextY += 40.f;
    }

    // ── Buttons ──────────────────────────────────────────────────────────────
    auto drawBtn = [&](const std::string& label, float bx, float by,
                        float bw, float bh, sf::Color bg, sf::Color col,
                        sf::FloatRect& outRect) {
        sf::RectangleShape btn(sf::Vector2f(bw, bh));
        btn.setPosition(bx, by);
        btn.setFillColor(bg);
        btn.setOutlineColor(sf::Color(150, 120, 60, 180));
        btn.setOutlineThickness(1.f);
        window.draw(btn);

        sf::Text t;
        t.setFont(m_assets.font());
        t.setCharacterSize(15);
        t.setFillColor(col);
        t.setString(label);
        auto bb = t.getLocalBounds();
        t.setOrigin(bb.left + bb.width / 2.f, bb.top + bb.height / 2.f);
        t.setPosition(bx + bw / 2.f, by + bh / 2.f);
        window.draw(t);

        outRect = sf::FloatRect(bx, by, bw, bh);
    };

    float btnH = 28.f;
    float contW = 160.f;
    float backW = 90.f;

    drawBtn("Continue  →",
            px + PW - contW - 20.f, nextY + 2.f, contW, btnH,
            sf::Color(30, 60, 20, 220), sf::Color(160, 230, 110),
            m_continueBtnRect);

    drawBtn("Back",
            px + 20.f, nextY + 2.f, backW, btnH,
            sf::Color(35, 28, 15, 210), sf::Color(180, 160, 110),
            m_backBtnRect);
}
