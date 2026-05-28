#include "gui/screens/ParamConfigScreen.h"
#include "algorithm/AlgoParams.h"
#include <SFML/Window/Keyboard.hpp>
#include <algorithm>
#include <stdexcept>

static constexpr float WIN_W = 1600.f;
static constexpr float WIN_H = 900.f;
static constexpr float IMG_W = 1672.f;
static constexpr float IMG_H = 941.f;

const char* ParamConfigScreen::algoName(int choice) {
    switch (choice) {
        case 1: return "Hill Climbing";
        case 2: return "Hill Climbing with Tries";
        case 3: return "Simulated Annealing";
        case 4: return "Genetic Algorithm";
        default: return "Unknown";
    }
}

ParamConfigScreen::ParamConfigScreen(AssetManager& assets, int algoChoice)
    : m_assets(assets), m_algoChoice(algoChoice)
{
    m_mapSprite.setTexture(m_assets.mapTexture());
    m_mapSprite.setScale(WIN_W / IMG_W, WIN_H / IMG_H);
    buildFields();
}

void ParamConfigScreen::buildFields() {
    m_fields.clear();
    switch (m_algoChoice) {
        case 2:  // HCT
            m_fields.push_back({"Max Tries  (t_max)",
                                "(default: 5 — uphill moves tolerated)",
                                "5", "5", false});
            break;

        case 3:  // SA
            m_fields.push_back({"Initial Temperature  (TI)",
                                "(default: 1000.0 — higher = more exploration)",
                                "1000.0", "1000.0", true});
            m_fields.push_back({"Final Temperature  (TF)",
                                "(default: 0.001 — stop criterion)",
                                "0.001", "0.001", true});
            m_fields.push_back({"Cooling Rate  (FR)",
                                "(default: 0.995 — range: 0.9–0.9999)",
                                "0.995", "0.995", true});
            break;

        case 4:  // GA
            m_fields.push_back({"Population Size",
                                "(default: 200)",
                                "200", "200", false});
            m_fields.push_back({"Max Generations",
                                "(default: 500)",
                                "500", "500", false});
            m_fields.push_back({"Mutation Rate  (%)",
                                "(default: 8 — probability as percentage)",
                                "8", "8", true});
            m_fields.push_back({"Tournament Size",
                                "(default: 3 — candidates per selection)",
                                "3", "3", false});
            m_fields.push_back({"Elite Fraction  (%)",
                                "(default: 10 — % of population kept unchanged)",
                                "10", "10", true});
            m_fields.push_back({"Breed Fraction  (%)",
                                "(default: 80 — % produced by crossover)",
                                "80", "80", true});
            m_fields.push_back({"Stagnation Limit",
                                "(default: 75 — generations without improvement)",
                                "75", "75", false});
            break;

        default:  // HC — no params
            break;
    }
    m_focused = -1;
    m_fieldRects.assign(m_fields.size(), sf::FloatRect{});
}

void ParamConfigScreen::resetToDefaults() {
    for (auto& f : m_fields)
        f.value = f.defaultVal;
    m_focused = -1;
}

static double safeDouble(const std::string& s, double fallback) {
    try { double v = std::stod(s); return (v > 0.0) ? v : fallback; }
    catch (...) { return fallback; }
}
static int safeInt(const std::string& s, int fallback, int lo = 1, int hi = 100000) {
    try { int v = std::stoi(s); return std::max(lo, std::min(hi, v)); }
    catch (...) { return fallback; }
}

void ParamConfigScreen::applyToData(TransitionData& data) const {
    switch (m_algoChoice) {
        case 2:
            data.hctTMax = safeInt(m_fields[0].value, 5, 1, 1000);
            break;
        case 3:
            data.saParams.TI = safeDouble(m_fields[0].value, 1000.0);
            data.saParams.TF = safeDouble(m_fields[1].value, 0.001);
            data.saParams.FR = safeDouble(m_fields[2].value, 0.995);
            data.saParams.FR = std::max(0.5, std::min(0.9999, data.saParams.FR));
            break;
        case 4:
            data.gaParams.popSize        = safeInt(m_fields[0].value,  200, 2, 5000);
            data.gaParams.generations    = safeInt(m_fields[1].value,  500, 1, 50000);
            data.gaParams.mutationRate   = safeDouble(m_fields[2].value, 8.0) / 100.0;
            data.gaParams.tournamentSize = safeInt(m_fields[3].value,  3, 1, 50);
            data.gaParams.gi             = safeDouble(m_fields[4].value, 10.0) / 100.0;
            data.gaParams.br             = safeDouble(m_fields[5].value, 80.0) / 100.0;
            data.gaParams.stagnation     = safeInt(m_fields[6].value,  75, 1, 50000);
            {
                double total = data.gaParams.gi + data.gaParams.br;
                if (total > 1.0) {
                    data.gaParams.gi /= total;
                    data.gaParams.br /= total;
                }
            }
            break;
        default:
            break;
    }
    data.benchmarkMode = m_benchmarkEnabled;
    data.benchmarkRuns = safeInt(m_runsStr, 20, 1, 500);
}

void ParamConfigScreen::handleEvent(const sf::Event& event, AppState& next, TransitionData& data) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            next = AppState::AlgoSelect;
            return;
        }
        if (event.key.code == sf::Keyboard::Tab) {
            if (!m_fields.empty())
                m_focused = (m_focused + 1) % (int)m_fields.size();
            m_runsFocused = false;
            return;
        }
        if (event.key.code == sf::Keyboard::Return) {
            applyToData(data);
            AppState gameOrBench = data.benchmarkMode ? AppState::Benchmark : AppState::Game;
            next = data.useRandom ? gameOrBench : AppState::NodeSelect;
            return;
        }
        if (event.key.code == sf::Keyboard::BackSpace) {
            if (m_runsFocused) {
                if (!m_runsStr.empty()) m_runsStr.pop_back();
            } else if (m_focused >= 0) {
                auto& val = m_fields[m_focused].value;
                if (!val.empty()) val.pop_back();
            }
        }
    }

    if (event.type == sf::Event::TextEntered) {
        char c = static_cast<char>(event.text.unicode);
        bool isDigit = (c >= '0' && c <= '9');
        if (m_runsFocused) {
            if (isDigit && m_runsStr.size() < 4) m_runsStr += c;
        } else if (m_focused >= 0) {
            auto& f = m_fields[m_focused];
            bool isDot  = (c == '.' && f.isFloat);
            bool hasDot = (f.value.find('.') != std::string::npos);
            if ((isDigit || (isDot && !hasDot)) && f.value.size() < 12)
                f.value += c;
        }
    }

    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f click((float)event.mouseButton.x, (float)event.mouseButton.y);

        m_focused = -1;
        m_runsFocused = false;

        for (int i = 0; i < (int)m_fieldRects.size(); i++) {
            if (m_fieldRects[i].contains(click)) { m_focused = i; return; }
        }

        if (m_benchmarkToggleRect.contains(click)) {
            m_benchmarkEnabled = !m_benchmarkEnabled;
            return;
        }
        if (m_benchmarkEnabled && m_runsBoxRect.contains(click)) {
            m_runsFocused = true;
            return;
        }

        if (m_resetBtnRect.contains(click))    { resetToDefaults(); return; }
        if (m_backBtnRect.contains(click))     { next = AppState::AlgoSelect; return; }
        if (m_continueBtnRect.contains(click)) {
            applyToData(data);
            AppState gameOrBench = data.benchmarkMode ? AppState::Benchmark : AppState::Game;
            next = data.useRandom ? gameOrBench : AppState::NodeSelect;
        }
    }
}

void ParamConfigScreen::update(float /*dt*/) {}

void ParamConfigScreen::draw(sf::RenderWindow& window) {
    window.draw(m_mapSprite);

    sf::RectangleShape overlay(sf::Vector2f(WIN_W, WIN_H));
    overlay.setFillColor(sf::Color(0, 0, 0, 128));
    window.draw(overlay);

    // Panel dimensions
    bool hasFields = !m_fields.empty();
    constexpr float ROW_H  = 56.f;
    constexpr float PW     = 620.f;
    float fieldH = hasFields ? m_fields.size() * ROW_H : 60.f;
    float PH = 110.f + fieldH + 60.f;
    float px = (WIN_W - PW) / 2.f;
    float py = (WIN_H - PH) / 2.f;

    sf::RectangleShape panel(sf::Vector2f(PW, PH));
    panel.setPosition(px, py);
    panel.setFillColor(sf::Color(10, 8, 6, 228));
    panel.setOutlineColor(sf::Color(150, 120, 60));
    panel.setOutlineThickness(2.f);
    window.draw(panel);

    // Title
    sf::Text title;
    title.setFont(m_assets.font());
    title.setCharacterSize(22);
    title.setFillColor(sf::Color(220, 200, 150));
    title.setStyle(sf::Text::Bold);
    title.setString("Parameters");
    auto tb = title.getLocalBounds();
    title.setOrigin(tb.left + tb.width / 2.f, tb.top);
    title.setPosition(px + PW / 2.f, py + 12.f);
    window.draw(title);

    sf::Text sub;
    sub.setFont(m_assets.font());
    sub.setCharacterSize(14);
    sub.setFillColor(sf::Color(160, 145, 110));
    sub.setString(algoName(m_algoChoice));
    auto sb = sub.getLocalBounds();
    sub.setOrigin(sb.left + sb.width / 2.f, sb.top);
    sub.setPosition(px + PW / 2.f, py + 42.f);
    window.draw(sub);

    sf::RectangleShape div(sf::Vector2f(PW - 20.f, 1.f));
    div.setPosition(px + 10.f, py + 64.f);
    div.setFillColor(sf::Color(100, 80, 40, 180));
    window.draw(div);

    float curY = py + 72.f;

    if (!hasFields) {
        // HC — no parameters
        sf::Text msg;
        msg.setFont(m_assets.font());
        msg.setCharacterSize(15);
        msg.setFillColor(sf::Color(160, 150, 120));
        msg.setString("Hill Climbing has no configurable parameters.");
        auto mb = msg.getLocalBounds();
        msg.setOrigin(mb.left + mb.width / 2.f, mb.top);
        msg.setPosition(px + PW / 2.f, curY + 14.f);
        window.draw(msg);
        curY += 60.f;
    } else {
        constexpr float LBL_W  = 240.f;
        constexpr float BOX_W  = 100.f;
        constexpr float BOX_H  = 28.f;
        constexpr float BOX_X  = 260.f;  // offset from panel left

        for (int i = 0; i < (int)m_fields.size(); i++) {
            const auto& f = m_fields[i];
            bool focused = (m_focused == i);
            float rowY = curY + i * ROW_H;

            // Row background for focused field
            if (focused) {
                sf::RectangleShape rowBg(sf::Vector2f(PW - 20.f, ROW_H - 4.f));
                rowBg.setPosition(px + 10.f, rowY);
                rowBg.setFillColor(sf::Color(28, 22, 10, 160));
                window.draw(rowBg);
            }

            // Label
            sf::Text lbl;
            lbl.setFont(m_assets.font());
            lbl.setCharacterSize(14);
            lbl.setFillColor(focused ? sf::Color(255, 240, 120) : sf::Color(200, 185, 150));
            lbl.setString(f.label);
            lbl.setPosition(px + 18.f, rowY + 8.f);
            window.draw(lbl);

            // Hint (small, below label)
            sf::Text hint;
            hint.setFont(m_assets.font());
            hint.setCharacterSize(11);
            hint.setFillColor(sf::Color(120, 110, 85));
            hint.setString(f.hint);
            hint.setPosition(px + 18.f, rowY + 28.f);
            window.draw(hint);

            // Input box
            float boxX = px + BOX_X;
            float boxY = rowY + (ROW_H - BOX_H) / 2.f;
            sf::RectangleShape box(sf::Vector2f(BOX_W, BOX_H));
            box.setPosition(boxX, boxY);
            box.setFillColor(sf::Color(20, 16, 8, 210));
            box.setOutlineColor(focused ? sf::Color(220, 185, 60, 240) :
                                          sf::Color(100, 80, 40, 180));
            box.setOutlineThickness(focused ? 2.f : 1.f);
            window.draw(box);

            std::string display = f.value.empty() ? "_" : f.value + (focused ? "_" : "");
            sf::Text inp;
            inp.setFont(m_assets.font());
            inp.setCharacterSize(14);
            inp.setFillColor(sf::Color(255, 240, 160));
            inp.setString(display);
            inp.setPosition(boxX + 6.f, boxY + 5.f);
            window.draw(inp);

            m_fieldRects[i] = sf::FloatRect(boxX, boxY, BOX_W, BOX_H);
        }
        curY += m_fields.size() * ROW_H + 4.f;
    }

    // ── Benchmark section ─────────────────────────────────────────────────────
    {
        sf::RectangleShape div2(sf::Vector2f(PW - 20.f, 1.f));
        div2.setPosition(px + 10.f, curY);
        div2.setFillColor(sf::Color(100, 80, 40, 140));
        window.draw(div2);
        curY += 8.f;

        // Toggle checkbox
        constexpr float BOX_SZ = 14.f;
        float bx = px + 12.f, by = curY + 2.f;
        sf::RectangleShape chk(sf::Vector2f(BOX_SZ, BOX_SZ));
        chk.setPosition(bx, by);
        chk.setFillColor(m_benchmarkEnabled ? sf::Color(50, 120, 50, 220) : sf::Color(20, 16, 8, 200));
        chk.setOutlineColor(sf::Color(140, 110, 50, 200));
        chk.setOutlineThickness(1.f);
        window.draw(chk);
        if (m_benchmarkEnabled) {
            sf::Text tick;
            tick.setFont(m_assets.font());
            tick.setCharacterSize(11);
            tick.setFillColor(sf::Color(160, 230, 130));
            tick.setString("x");
            tick.setPosition(bx + 3.f, by + 1.f);
            window.draw(tick);
        }
        m_benchmarkToggleRect = sf::FloatRect(bx, by, BOX_SZ + 160.f, BOX_SZ);

        sf::Text lbl;
        lbl.setFont(m_assets.font());
        lbl.setCharacterSize(13);
        lbl.setFillColor(m_benchmarkEnabled ? sf::Color(200, 230, 160) : sf::Color(160, 150, 120));
        lbl.setString("Performance Test mode");
        lbl.setPosition(bx + BOX_SZ + 6.f, by);
        window.draw(lbl);

        if (m_benchmarkEnabled) {
            sf::Text runsLbl;
            runsLbl.setFont(m_assets.font());
            runsLbl.setCharacterSize(13);
            runsLbl.setFillColor(sf::Color(160, 145, 110));
            runsLbl.setString("Runs:");
            runsLbl.setPosition(px + PW - 170.f, curY + 2.f);
            window.draw(runsLbl);

            float rbx = px + PW - 120.f, rby = curY;
            constexpr float RBW = 55.f, RBH = 20.f;
            sf::RectangleShape rb(sf::Vector2f(RBW, RBH));
            rb.setPosition(rbx, rby);
            rb.setFillColor(sf::Color(20, 16, 8, 210));
            rb.setOutlineColor(m_runsFocused ? sf::Color(200, 180, 60, 220) : sf::Color(100, 80, 40, 160));
            rb.setOutlineThickness(m_runsFocused ? 2.f : 1.f);
            window.draw(rb);
            m_runsBoxRect = sf::FloatRect(rbx, rby, RBW, RBH);

            std::string disp = m_runsStr.empty() ? "_" : m_runsStr + (m_runsFocused ? "_" : "");
            sf::Text rv;
            rv.setFont(m_assets.font());
            rv.setCharacterSize(13);
            rv.setFillColor(sf::Color(255, 240, 160));
            rv.setString(disp);
            rv.setPosition(rbx + 5.f, rby + 2.f);
            window.draw(rv);
        }
        curY += 24.f;
    }
    // ─────────────────────────────────────────────────────────────────────────

    // Divider above buttons
    sf::RectangleShape div3(sf::Vector2f(PW - 20.f, 1.f));
    div3.setPosition(px + 10.f, curY);
    div3.setFillColor(sf::Color(100, 80, 40, 180));
    window.draw(div3);
    curY += 8.f;

    auto drawBtn = [&](const std::string& lbl, float bx, float by, float bw, float bh,
                        sf::Color bg, sf::Color col, sf::FloatRect& outRect) {
        sf::RectangleShape b(sf::Vector2f(bw, bh));
        b.setPosition(bx, by);
        b.setFillColor(bg);
        b.setOutlineColor(sf::Color(150, 120, 60, 160));
        b.setOutlineThickness(1.f);
        window.draw(b);
        sf::Text t;
        t.setFont(m_assets.font());
        t.setCharacterSize(14);
        t.setFillColor(col);
        t.setString(lbl);
        auto tb2 = t.getLocalBounds();
        t.setOrigin(tb2.left + tb2.width / 2.f, tb2.top + tb2.height / 2.f);
        t.setPosition(bx + bw / 2.f, by + bh / 2.f);
        window.draw(t);
        outRect = sf::FloatRect(bx, by, bw, bh);
    };

    float btnH  = 28.f;
    float btnY  = curY + 4.f;

    drawBtn("Back",
            px + 10.f, btnY, 90.f, btnH,
            sf::Color(35, 28, 15, 210), sf::Color(180, 160, 110),
            m_backBtnRect);

    if (hasFields)
        drawBtn("Reset to Defaults",
                px + 110.f, btnY, 160.f, btnH,
                sf::Color(50, 30, 10, 210), sf::Color(200, 160, 80),
                m_resetBtnRect);
    else
        m_resetBtnRect = {};

    drawBtn("Continue  →",
            px + PW - 150.f, btnY, 140.f, btnH,
            sf::Color(30, 60, 20, 220), sf::Color(160, 230, 110),
            m_continueBtnRect);
}
