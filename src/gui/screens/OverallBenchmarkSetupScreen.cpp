#include "gui/screens/OverallBenchmarkSetupScreen.h"
#include <SFML/Window/Keyboard.hpp>
#include <algorithm>
#include <stdexcept>

static constexpr float WIN_W = 1600.f;
static constexpr float WIN_H = 900.f;
static constexpr float IMG_W = 1672.f;
static constexpr float IMG_H = 941.f;

double OverallBenchmarkSetupScreen::safeDouble(const std::string& s, double fallback) {
    try { double v = std::stod(s); return (v > 0.0) ? v : fallback; }
    catch (...) { return fallback; }
}
int OverallBenchmarkSetupScreen::safeInt(const std::string& s, int fallback, int lo, int hi) {
    try { int v = std::stoi(s); return std::max(lo, std::min(hi, v)); }
    catch (...) { return fallback; }
}

OverallBenchmarkSetupScreen::OverallBenchmarkSetupScreen(AssetManager& assets)
    : m_assets(assets)
{
    m_mapSprite.setTexture(m_assets.mapTexture());
    m_mapSprite.setScale(WIN_W / IMG_W, WIN_H / IMG_H);
}

void OverallBenchmarkSetupScreen::applyToData(TransitionData& data) const {
    data.overallBenchmark = true;
    data.benchmarkRuns    = safeInt(m_runsStr, 20, 1, 500);
    data.useRandom        = m_useRandom;
    data.randomCount      = safeInt(m_nodeCountStr, 20, 5, 100);
    data.fixedStart       = m_fixedNone ? -1 : safeInt(m_startNodeStr, 0, 0, 99);

    data.hctTMax = safeInt(m_hctTMaxStr, 5, 1, 1000);

    data.saParams.TI = safeDouble(m_saTIStr, 1000.0);
    data.saParams.TF = safeDouble(m_saTFStr, 0.001);
    data.saParams.FR = safeDouble(m_saFRStr, 0.995);
    data.saParams.FR = std::max(0.5, std::min(0.9999, data.saParams.FR));

    data.gaParams.popSize        = safeInt(m_gaPopStr,   200, 2, 5000);
    data.gaParams.generations    = safeInt(m_gaGenStr,   500, 1, 50000);
    data.gaParams.mutationRate   = safeDouble(m_gaMutStr,   8.0) / 100.0;
    data.gaParams.tournamentSize = safeInt(m_gaTourStr,    3, 1, 50);
    data.gaParams.gi             = safeDouble(m_gaEliteStr, 10.0) / 100.0;
    data.gaParams.br             = safeDouble(m_gaBreedStr, 80.0) / 100.0;
    data.gaParams.stagnation     = safeInt(m_gaStagStr,   75, 1, 50000);
    double total = data.gaParams.gi + data.gaParams.br;
    if (total > 1.0) { data.gaParams.gi /= total; data.gaParams.br /= total; }
}

void OverallBenchmarkSetupScreen::handleEvent(const sf::Event& event, AppState& next, TransitionData& data) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) { next = AppState::MainMenu; return; }
        if (event.key.code == sf::Keyboard::Tab) {
            m_focused = (m_focused + 1) % (int)m_inputFields.size();
            return;
        }
        if (event.key.code == sf::Keyboard::Return) {
            applyToData(data);
            next = AppState::OverallBenchmark;
            return;
        }
        if (event.key.code == sf::Keyboard::BackSpace && m_focused >= 0) {
            auto& val = *m_inputFields[m_focused].value;
            if (!val.empty()) val.pop_back();
        }
    }

    if (event.type == sf::Event::TextEntered && m_focused >= 0) {
        char c = static_cast<char>(event.text.unicode);
        auto& f = m_inputFields[m_focused];
        bool isDigit = (c >= '0' && c <= '9');
        bool isDot   = (c == '.' && f.isFloat);
        bool hasDot  = (f.value->find('.') != std::string::npos);
        if ((isDigit || (isDot && !hasDot)) && f.value->size() < 10)
            *f.value += c;
    }

    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f click((float)event.mouseButton.x, (float)event.mouseButton.y);

        if (m_defaultGraphRect.contains(click)) { m_useRandom = false; m_focused = -1; return; }
        if (m_randomGraphRect.contains(click))  { m_useRandom = true;  m_focused = -1; return; }
        if (m_fixedStartToggleRect.contains(click)) { m_fixedNone = !m_fixedNone; m_focused = -1; return; }
        if (m_backBtnRect.contains(click))  { next = AppState::MainMenu; return; }
        if (m_startBtnRect.contains(click)) {
            applyToData(data);
            next = AppState::OverallBenchmark;
            return;
        }

        m_focused = -1;
        for (int i = 0; i < (int)m_inputFields.size(); i++) {
            if (m_inputFields[i].rect.contains(click)) { m_focused = i; return; }
        }
    }
}

void OverallBenchmarkSetupScreen::update(float /*dt*/) {}

void OverallBenchmarkSetupScreen::draw(sf::RenderWindow& window) {
    window.draw(m_mapSprite);

    sf::RectangleShape overlay(sf::Vector2f(WIN_W, WIN_H));
    overlay.setFillColor(sf::Color(0, 0, 0, 130));
    window.draw(overlay);

    constexpr float PW = 700.f;
    constexpr float PH = 600.f;
    float px = (WIN_W - PW) / 2.f;
    float py = (WIN_H - PH) / 2.f;

    sf::RectangleShape panel(sf::Vector2f(PW, PH));
    panel.setPosition(px, py);
    panel.setFillColor(sf::Color(10, 8, 6, 228));
    panel.setOutlineColor(sf::Color(150, 120, 60));
    panel.setOutlineThickness(2.f);
    window.draw(panel);

    // Rebuild input field list each frame for hit-testing
    m_inputFields.clear();

    // --- Helpers ---
    auto makeText = [&](const std::string& s, unsigned sz, sf::Color c, bool bold = false) {
        sf::Text t;
        t.setFont(m_assets.font());
        t.setCharacterSize(sz);
        t.setFillColor(c);
        if (bold) t.setStyle(sf::Text::Bold);
        t.setString(s);
        return t;
    };
    auto drawDivider = [&](float y) {
        sf::RectangleShape d(sf::Vector2f(PW - 20.f, 1.f));
        d.setPosition(px + 10.f, y);
        d.setFillColor(sf::Color(100, 80, 40, 160));
        window.draw(d);
    };
    auto drawSecHdr = [&](const std::string& s, float y) {
        auto t = makeText(s, 11, sf::Color(180, 155, 90), true);
        t.setPosition(px + 12.f, y);
        window.draw(t);
    };

    // Input box helper: draws box, text, registers hit rect
    auto drawInputBox = [&](std::string& val, bool isFloat,
                             float bx, float by, float bw, float bh = 22.f) {
        int idx = (int)m_inputFields.size();
        bool focused = (m_focused == idx);
        sf::RectangleShape box(sf::Vector2f(bw, bh));
        box.setPosition(bx, by);
        box.setFillColor(sf::Color(20, 16, 8, 210));
        box.setOutlineColor(focused ? sf::Color(220, 185, 60, 240) : sf::Color(100, 80, 40, 160));
        box.setOutlineThickness(focused ? 2.f : 1.f);
        window.draw(box);
        std::string disp = val.empty() ? "_" : val + (focused ? "_" : "");
        auto vt = makeText(disp, 12, sf::Color(255, 240, 160));
        vt.setPosition(bx + 4.f, by + 3.f);
        window.draw(vt);
        m_inputFields.push_back({&val, isFloat, sf::FloatRect(bx, by, bw, bh)});
    };

    // Button helper
    auto drawButton = [&](const std::string& label, float bx, float by, float bw, float bh,
                          sf::Color bg, sf::Color fg, sf::FloatRect& outRect) {
        sf::RectangleShape b(sf::Vector2f(bw, bh));
        b.setPosition(bx, by);
        b.setFillColor(bg);
        b.setOutlineColor(sf::Color(150, 120, 60, 160));
        b.setOutlineThickness(1.f);
        window.draw(b);
        auto t = makeText(label, 13, fg);
        auto tb = t.getLocalBounds();
        t.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
        t.setPosition(bx + bw / 2.f, by + bh / 2.f);
        window.draw(t);
        outRect = sf::FloatRect(bx, by, bw, bh);
    };

    // --- Title ---
    float y = py + 10.f;
    {
        auto title = makeText("Overall Benchmark Setup", 17, sf::Color(220, 200, 150), true);
        auto tb = title.getLocalBounds();
        title.setOrigin(tb.left + tb.width / 2.f, tb.top);
        title.setPosition(px + PW / 2.f, y);
        window.draw(title);
        y += 26.f;
    }
    {
        auto sub = makeText("Runs all 4 algorithms N times each, then shows a comparison table.", 12, sf::Color(150, 140, 110));
        auto sb = sub.getLocalBounds();
        sub.setOrigin(sb.left + sb.width / 2.f, sb.top);
        sub.setPosition(px + PW / 2.f, y);
        window.draw(sub);
        y += 20.f;
    }
    drawDivider(y); y += 10.f;

    // --- GENERAL section ---
    drawSecHdr("GENERAL", y); y += 18.f;

    // Runs per algorithm
    {
        auto lbl = makeText("Runs per algorithm:", 13, sf::Color(160, 145, 110));
        lbl.setPosition(px + 14.f, y + 2.f);
        window.draw(lbl);
        drawInputBox(m_runsStr, false, px + 210.f, y, 55.f);
        y += 24.f;
    }

    // Graph type
    {
        auto lbl = makeText("Graph:", 13, sf::Color(160, 145, 110));
        lbl.setPosition(px + 14.f, y + 4.f);
        window.draw(lbl);

        float bx = px + 90.f, by = y;
        drawButton("Default WorldMap", bx, by, 155.f, 22.f,
            m_useRandom ? sf::Color(25, 20, 10, 200) : sf::Color(20, 55, 85, 220),
            m_useRandom ? sf::Color(160, 150, 120) : sf::Color(160, 200, 240),
            m_defaultGraphRect);
        drawButton("Random Graph", bx + 162.f, by, 130.f, 22.f,
            m_useRandom ? sf::Color(20, 55, 85, 220) : sf::Color(25, 20, 10, 200),
            m_useRandom ? sf::Color(160, 200, 240) : sf::Color(160, 150, 120),
            m_randomGraphRect);

        if (m_useRandom) {
            auto nc = makeText("Nodes:", 13, sf::Color(160, 145, 110));
            nc.setPosition(bx + 300.f, y + 2.f);
            window.draw(nc);
            drawInputBox(m_nodeCountStr, false, bx + 355.f, by, 50.f);
        }
        y += 28.f;
    }

    // Fixed start
    {
        auto lbl = makeText("Fixed Start:", 13, sf::Color(160, 145, 110));
        lbl.setPosition(px + 14.f, y + 3.f);
        window.draw(lbl);

        // Checkbox "None (random per run)"
        float cbx = px + 110.f, cby = y + 3.f;
        sf::RectangleShape chk(sf::Vector2f(14.f, 14.f));
        chk.setPosition(cbx, cby);
        chk.setFillColor(m_fixedNone ? sf::Color(50, 120, 50, 220) : sf::Color(20, 16, 8, 200));
        chk.setOutlineColor(sf::Color(140, 110, 50, 200));
        chk.setOutlineThickness(1.f);
        window.draw(chk);
        if (m_fixedNone) {
            auto tick = makeText("x", 11, sf::Color(160, 230, 130));
            tick.setPosition(cbx + 3.f, cby + 1.f);
            window.draw(tick);
        }
        m_fixedStartToggleRect = sf::FloatRect(cbx, cby, 150.f, 14.f);

        auto none_lbl = makeText("None (random per run)", 13,
            m_fixedNone ? sf::Color(200, 220, 160) : sf::Color(160, 150, 120));
        none_lbl.setPosition(cbx + 18.f, y + 2.f);
        window.draw(none_lbl);

        if (!m_fixedNone) {
            auto id_lbl = makeText("Node ID:", 13, sf::Color(160, 145, 110));
            id_lbl.setPosition(px + 320.f, y + 2.f);
            window.draw(id_lbl);
            drawInputBox(m_startNodeStr, false, px + 400.f, y, 50.f);
        }
        y += 26.f;
    }

    y += 4.f;
    drawDivider(y); y += 10.f;

    // --- ALGORITHM PARAMETERS section ---
    drawSecHdr("ALGORITHM PARAMETERS  (edit to override defaults)", y); y += 18.f;

    // Row helper: label + one or more input boxes on the same line
    auto drawParamRow = [&](const std::string& algoLabel, float ly) {
        auto t = makeText(algoLabel, 12, sf::Color(200, 180, 120), true);
        t.setPosition(px + 14.f, ly);
        window.draw(t);
    };

    // HC
    drawParamRow("Hill Climbing", y);
    {
        auto t = makeText("no configurable parameters", 12, sf::Color(120, 110, 85));
        t.setPosition(px + 165.f, y);
        window.draw(t);
        y += 18.f;
    }

    // HCT
    drawParamRow("HC with Tries", y);
    {
        auto lbl = makeText("Max Tries:", 12, sf::Color(160, 145, 110));
        lbl.setPosition(px + 165.f, y + 2.f);
        window.draw(lbl);
        drawInputBox(m_hctTMaxStr, false, px + 255.f, y, 50.f, 20.f);
        y += 22.f;
    }

    // SA
    drawParamRow("Simulated Annealing", y);
    {
        struct { const char* lbl; std::string* val; bool isFloat; } sa_fields[3] = {
            {"TI:", &m_saTIStr, true}, {"TF:", &m_saTFStr, true}, {"FR:", &m_saFRStr, true}
        };
        float fx = px + 210.f;
        for (auto& f : sa_fields) {
            auto t = makeText(f.lbl, 12, sf::Color(160, 145, 110));
            t.setPosition(fx, y + 2.f);
            window.draw(t);
            fx += 24.f;
            drawInputBox(*f.val, f.isFloat, fx, y, 75.f, 20.f);
            fx += 82.f;
        }
        y += 22.f;
    }

    // GA row 1
    drawParamRow("Genetic Algorithm", y);
    {
        struct { const char* lbl; std::string* val; } ga1[4] = {
            {"Pop:", &m_gaPopStr}, {"Gens:", &m_gaGenStr},
            {"Mut%:", &m_gaMutStr}, {"Tour:", &m_gaTourStr}
        };
        float fx = px + 210.f;
        for (auto& f : ga1) {
            auto t = makeText(f.lbl, 12, sf::Color(160, 145, 110));
            t.setPosition(fx, y + 2.f);
            window.draw(t);
            fx += (std::string(f.lbl).size() + 1) * 7.5f;
            drawInputBox(*f.val, false, fx, y, 50.f, 20.f);
            fx += 56.f;
        }
        y += 22.f;
    }

    // GA row 2
    {
        struct { const char* lbl; std::string* val; } ga2[3] = {
            {"Elite%:", &m_gaEliteStr}, {"Breed%:", &m_gaBreedStr}, {"Stag:", &m_gaStagStr}
        };
        float fx = px + 210.f;
        for (auto& f : ga2) {
            auto t = makeText(f.lbl, 12, sf::Color(160, 145, 110));
            t.setPosition(fx, y + 2.f);
            window.draw(t);
            fx += (std::string(f.lbl).size() + 1) * 7.5f;
            drawInputBox(*f.val, false, fx, y, 50.f, 20.f);
            fx += 56.f;
        }
        y += 22.f;
    }

    y += 6.f;
    drawDivider(y); y += 10.f;

    // --- Buttons ---
    float btnH = 30.f;
    float btnY = y + 4.f;
    drawButton("Back", px + 12.f, btnY, 90.f, btnH,
               sf::Color(35, 28, 15, 210), sf::Color(180, 160, 110), m_backBtnRect);
    drawButton("Start Overall Benchmark  ->", px + PW - 250.f, btnY, 238.f, btnH,
               sf::Color(30, 60, 20, 220), sf::Color(160, 230, 110), m_startBtnRect);
}
