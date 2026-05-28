#include "gui/screens/BenchmarkScreen.h"
#include "core/Node.h"
#include "core/Edge.h"
#include <SFML/Window/Keyboard.hpp>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <numeric>

static constexpr float WIN_W  = 1600.f;
static constexpr float WIN_H  = 900.f;
static constexpr float IMG_W  = 1672.f;
static constexpr float IMG_H  = 941.f;
static constexpr float NODE_R = 8.f;
static constexpr float FRAME_DURATION = 0.05f;

static constexpr float STATS_PW = 380.f;
static constexpr float STATS_PH = WIN_H - 40.f;
static constexpr float STATS_PX = WIN_W - STATS_PW - 10.f;
static constexpr float STATS_PY = 20.f;

static const sf::Color COL_EDGE_ROAD  = sf::Color(190, 150,  80, 160);
static const sf::Color COL_ANIM_PATH  = sf::Color(230,  50,  50, 220);
static const sf::Color COL_FINAL_PATH = sf::Color(245, 215,  40, 240);
static const sf::Color COL_NODE_FILL  = sf::Color(230, 220, 190);
static const sf::Color COL_NODE_OUT   = sf::Color( 40,  25,  10);

static void drawThickLine(sf::RenderWindow& window,
                          sf::Vector2f a, sf::Vector2f b,
                          sf::Color color, float thickness) {
    sf::Vector2f dir = b - a;
    float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
    if (len < 0.5f) return;
    dir /= len;
    sf::Vector2f perp(-dir.y * thickness * 0.5f, dir.x * thickness * 0.5f);
    sf::VertexArray q(sf::Quads, 4);
    q[0].position = a + perp; q[1].position = b + perp;
    q[2].position = b - perp; q[3].position = a - perp;
    q[0].color = q[1].color = q[2].color = q[3].color = color;
    window.draw(q);
}

static double medianOf(std::vector<double> v) {
    if (v.empty()) return 0.0;
    std::sort(v.begin(), v.end());
    int n = (int)v.size();
    return (n % 2 == 0) ? (v[n/2 - 1] + v[n/2]) * 0.5 : v[n/2];
}

BenchmarkScreen::BenchmarkScreen(AssetManager& assets, const Graph& graph, const TSP& tsp,
                                 NodePositions positions,
                                 int algoChoice, int hctTMax, int fixedStart,
                                 SAParams saParams, GAParams gaParams,
                                 int numRuns)
    : m_assets(assets), m_graph(graph), m_positions(std::move(positions)),
      m_algoChoice(algoChoice), m_hctTMax(hctTMax), m_fixedStart(fixedStart),
      m_saParams(saParams), m_gaParams(gaParams),
      m_numRuns(std::max(1, numRuns)), m_tsp(tsp)
{
    m_mapSprite.setTexture(m_assets.mapTexture());
    m_mapView = sf::View(sf::FloatRect(0.f, 0.f, IMG_W, IMG_H));

    m_backBtn.setFont(m_assets.font());
    m_backBtn.setString("[ Back ]");
    m_backBtn.setCharacterSize(18);
    m_backBtn.setFillColor(sf::Color(200, 180, 120));
    m_backBtn.setPosition(12.f, WIN_H - 30.f);

    m_zoomHint.setFont(m_assets.font());
    m_zoomHint.setCharacterSize(13);
    m_zoomHint.setFillColor(sf::Color(160, 150, 120, 180));
    m_zoomHint.setString("Scroll: zoom  |  Right-drag: pan  |  Scroll panel: list  |  ESC: back");
    m_zoomHint.setPosition(130.f, WIN_H - 26.f);

    if (m_graph.size() == NodePositions::NODE_COUNT && !m_positions.allPlaced())
        m_positions.applyFallback();

    m_distMatrix = m_graph.allPairsShortestPath();
    m_initialCosts.reserve(m_numRuns);
    m_finalCosts.reserve(m_numRuns);

    startRun();
}

void BenchmarkScreen::startRun() {
    m_runner.run(m_algoChoice, m_hctTMax, m_tsp, m_graph.size(),
                 m_fixedStart, 250, m_saParams, m_gaParams);
    m_currentRun++;
    m_frameIdx   = 0;
    m_animTimer  = 0.f;
    m_animDone   = false;

    m_initialCosts.push_back(m_runner.initialCost());
    m_finalCosts.push_back(m_runner.bestCost());
}

void BenchmarkScreen::computeMedians() {
    m_medianInitial = medianOf(m_initialCosts);
    m_medianFinal   = medianOf(m_finalCosts);
    m_medianGain    = (m_medianInitial > 0.0)
        ? 100.0 * std::abs(m_medianInitial - m_medianFinal) / m_medianInitial
        : 0.0;
    m_allDone = true;
}

void BenchmarkScreen::handleEvent(const sf::Event& event, AppState& next, TransitionData& data) {
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
        next = AppState::MainMenu;

    if (event.type == sf::Event::MouseButtonPressed) {
        sf::Vector2f click(event.mouseButton.x, event.mouseButton.y);
        if (event.mouseButton.button == sf::Mouse::Left &&
            m_backBtn.getGlobalBounds().contains(click))
            next = AppState::MainMenu;
        if (event.mouseButton.button == sf::Mouse::Right) {
            m_panning = true;
            m_panStart = {event.mouseButton.x, event.mouseButton.y};
        }
    }
    if (event.type == sf::Event::MouseButtonReleased &&
        event.mouseButton.button == sf::Mouse::Right)
        m_panning = false;

    if (event.type == sf::Event::MouseMoved) {
        m_mousePos = sf::Vector2f(event.mouseMove.x, event.mouseMove.y);
        if (m_panning) {
            sf::Vector2i curr(event.mouseMove.x, event.mouseMove.y);
            sf::Vector2f vs = m_mapView.getSize();
            m_mapView.move({(float)(m_panStart.x - curr.x) * vs.x / WIN_W,
                            (float)(m_panStart.y - curr.y) * vs.y / WIN_H});
            m_panStart = curr;
        }
    }

    if (event.type == sf::Event::MouseWheelScrolled) {
        float mx = event.mouseWheelScroll.x, my = event.mouseWheelScroll.y;
        bool overPanel = m_allDone &&
            mx >= STATS_PX && mx <= STATS_PX + STATS_PW &&
            my >= STATS_PY && my <= STATS_PY + STATS_PH;

        if (overPanel) {
            m_statsScroll -= (int)event.mouseWheelScroll.delta;
            if (m_statsScroll < 0) m_statsScroll = 0;
        } else {
            float factor = (event.mouseWheelScroll.delta > 0) ? 0.85f : 1.f / 0.85f;
            float nz = m_currentZoom * factor;
            if (nz >= 0.1f && nz <= 3.f) {
                m_currentZoom = nz;
                sf::Vector2f before = m_mapView.getCenter()
                    + sf::Vector2f((mx - WIN_W*0.5f)*m_mapView.getSize().x/WIN_W,
                                   (my - WIN_H*0.5f)*m_mapView.getSize().y/WIN_H);
                m_mapView.zoom(factor);
                sf::Vector2f after = m_mapView.getCenter()
                    + sf::Vector2f((mx - WIN_W*0.5f)*m_mapView.getSize().x/WIN_W,
                                   (my - WIN_H*0.5f)*m_mapView.getSize().y/WIN_H);
                m_mapView.move(before - after);
            }
        }
    }

    (void)data;
}

void BenchmarkScreen::update(float dt) {
    if (m_allDone) return;
    m_animTimer += dt;
    if (m_animTimer >= FRAME_DURATION) {
        m_animTimer -= FRAME_DURATION;
        if (m_frameIdx + 1 < m_runner.frames().size())
            m_frameIdx++;
        else
            m_animDone = true;
    }

    if (m_animDone) {
        if (m_currentRun < m_numRuns)
            startRun();
        else
            computeMedians();
    }
}

void BenchmarkScreen::drawEdges(sf::RenderWindow& window) {
    int n = m_graph.size();
    for (int i = 0; i < n-1; i++)
        for (int j = i+1; j < n; j++) {
            if (!m_graph.hasEdge(i,j)) continue;
            if (!m_positions.isPlaced(i) || !m_positions.isPlaced(j)) continue;
            drawThickLine(window, m_positions.get(i), m_positions.get(j), COL_EDGE_ROAD, 2.f);
        }
}

void BenchmarkScreen::drawPath(sf::RenderWindow& window, const std::vector<int>& route,
                               sf::Color color, float thickness) {
    int n = (int)route.size();
    for (int i = 0; i < n; i++) {
        int a = route[i], b = route[(i+1)%n];
        if (!m_positions.isPlaced(a) || !m_positions.isPlaced(b)) continue;
        drawThickLine(window, m_positions.get(a), m_positions.get(b), color, thickness);
    }
}

void BenchmarkScreen::drawNodes(sf::RenderWindow& window, const std::vector<int>& highlight,
                                sf::Color pathColor, bool showOrder) {
    int n = m_graph.size();
    sf::CircleShape circle(NODE_R);
    circle.setOrigin(NODE_R, NODE_R);
    circle.setOutlineThickness(2.f);
    circle.setOutlineColor(COL_NODE_OUT);

    std::vector<int> visitOrder(n, -1);
    if (showOrder)
        for (int i = 0; i < (int)highlight.size(); i++)
            visitOrder[highlight[i]] = i + 1;

    sf::Text label;
    label.setFont(m_assets.font());
    label.setCharacterSize(8);
    int startNode = (!highlight.empty() && showOrder) ? highlight[0] : -1;

    for (int i = 0; i < n; i++) {
        if (!m_positions.isPlaced(i)) continue;
        sf::Vector2f pos = m_positions.get(i);
        bool inPath = false;
        for (int h : highlight) if (h == i) { inPath = true; break; }
        bool isStart = (i == startNode);
        float r = isStart ? NODE_R * 1.8f : NODE_R;
        circle.setRadius(r); circle.setOrigin(r, r); circle.setPosition(pos);
        circle.setOutlineThickness(isStart ? 3.f : 2.f);
        circle.setOutlineColor(isStart ? sf::Color(255,255,255,240) : COL_NODE_OUT);
        circle.setFillColor(isStart ? sf::Color(255,220,60) :
                            (inPath && showOrder) ? pathColor : COL_NODE_FILL);
        window.draw(circle);
        if (showOrder && visitOrder[i] >= 0) {
            label.setString(std::to_string(visitOrder[i]));
            label.setFillColor(sf::Color(255,255,255,230));
            label.setPosition(pos.x+r+1.f, pos.y-10.f);
            window.draw(label);
        }
        label.setString(m_graph.getNode(i).name.substr(0, 14));
        label.setFillColor(sf::Color(240,230,200,170));
        label.setPosition(pos.x+r+1.f, pos.y+2.f);
        window.draw(label);
    }
}

void BenchmarkScreen::drawProgress(sf::RenderWindow& window) {
    constexpr float PW = 280.f, PH = 110.f;
    float px = WIN_W - STATS_PW - PW - 20.f;
    float py = 20.f;

    sf::RectangleShape panel(sf::Vector2f(PW, PH));
    panel.setPosition(px, py);
    panel.setFillColor(sf::Color(10, 8, 6, 215));
    panel.setOutlineColor(sf::Color(150, 120, 60));
    panel.setOutlineThickness(1.5f);
    window.draw(panel);

    auto txt = [&](const std::string& s, unsigned sz, sf::Color c, float x, float y) {
        sf::Text t;
        t.setFont(m_assets.font());
        t.setCharacterSize(sz);
        t.setFillColor(c);
        t.setString(s);
        t.setPosition(x, y);
        window.draw(t);
    };

    txt("Performance Test", 14, sf::Color(220, 200, 150), px+10.f, py+8.f);

    // Progress bar
    float barW = PW - 20.f;
    float filled = m_numRuns > 0 ? (float)(m_currentRun - 1) / m_numRuns * barW : 0.f;
    sf::RectangleShape barBg(sf::Vector2f(barW, 10.f));
    barBg.setPosition(px+10.f, py+30.f);
    barBg.setFillColor(sf::Color(35, 28, 15, 200));
    barBg.setOutlineColor(sf::Color(100, 80, 40, 160));
    barBg.setOutlineThickness(1.f);
    window.draw(barBg);
    if (filled > 0.f) {
        sf::RectangleShape barFill(sf::Vector2f(filled, 10.f));
        barFill.setPosition(px+10.f, py+30.f);
        barFill.setFillColor(sf::Color(80, 160, 80, 220));
        window.draw(barFill);
    }

    std::ostringstream ss;
    ss << "Run " << m_currentRun << " / " << m_numRuns;
    txt(ss.str(), 13, sf::Color(200, 190, 155), px+10.f, py+48.f);

    ss.str(""); ss << std::fixed << std::setprecision(1)
                   << "Current best: " << m_runner.bestCost();
    txt(ss.str(), 12, sf::Color(170, 160, 130), px+10.f, py+66.f);

    if (m_currentRun > 1) {
        double med = medianOf(m_finalCosts);
        ss.str(""); ss << "Median so far: " << med;
        txt(ss.str(), 12, sf::Color(150, 200, 150), px+10.f, py+84.f);
    }
}

void BenchmarkScreen::drawFinalStats(sf::RenderWindow& window) {
    sf::RectangleShape panel(sf::Vector2f(STATS_PW, STATS_PH));
    panel.setPosition(STATS_PX, STATS_PY);
    panel.setFillColor(sf::Color(10, 8, 6, 218));
    panel.setOutlineColor(sf::Color(150, 120, 60));
    panel.setOutlineThickness(1.5f);
    window.draw(panel);

    auto makeText = [&](const std::string& s, unsigned sz, sf::Color col, bool bold = false) {
        sf::Text t;
        t.setFont(m_assets.font());
        t.setCharacterSize(sz);
        t.setFillColor(col);
        if (bold) t.setStyle(sf::Text::Bold);
        t.setString(s);
        return t;
    };
    auto drawDivider = [&](float y) {
        sf::RectangleShape d(sf::Vector2f(STATS_PW - 16.f, 1.f));
        d.setPosition(STATS_PX + 8.f, y);
        d.setFillColor(sf::Color(100, 80, 40, 180));
        window.draw(d);
    };
    auto drawRow = [&](const std::string& lbl, const std::string& val, float y) {
        auto lt = makeText(lbl, 13, sf::Color(160, 145, 110));
        lt.setPosition(STATS_PX + 10.f, y);
        window.draw(lt);
        auto vt = makeText(val, 13, sf::Color(230, 220, 180));
        vt.setPosition(STATS_PX + 158.f, y);
        window.draw(vt);
    };
    auto drawSecHdr = [&](const std::string& s, float y) {
        auto t = makeText(s, 11, sf::Color(180, 155, 90), true);
        t.setPosition(STATS_PX + 10.f, y);
        window.draw(t);
    };

    float y = STATS_PY + 8.f;

    // Title
    {
        auto title = makeText("Performance Test Results", 14, sf::Color(220, 200, 150), true);
        auto tb = title.getLocalBounds();
        title.setOrigin(tb.left + tb.width/2.f, tb.top);
        title.setPosition(STATS_PX + STATS_PW/2.f, y);
        window.draw(title);
        y += 24.f;
    }
    drawDivider(y); y += 10.f;

    // Configuration
    drawSecHdr("CONFIGURATION", y); y += 18.f;
    static const char* ALGO_NAMES[] = {"","Hill Climbing","HC with Tries","Simulated Annealing","Genetic Algorithm"};
    drawRow("Algorithm:", ALGO_NAMES[m_algoChoice], y); y += 18.f;
    drawRow("Nodes:", std::to_string(m_graph.size()), y); y += 18.f;
    drawRow("Runs:", std::to_string(m_numRuns), y); y += 18.f;
    if (m_fixedStart >= 0) {
        std::string sn = m_graph.getNode(m_fixedStart).name;
        if ((int)sn.size() > 16) sn = sn.substr(0, 15) + ".";
        drawRow("Fixed Start:", sn, y);
    } else {
        drawRow("Fixed Start:", "None", y);
    }
    y += 18.f;

    // Algo params
    std::ostringstream pss;
    pss << std::fixed;
    if (m_algoChoice == 2) {
        drawRow("Max Tries:", std::to_string(m_hctTMax), y); y += 15.f;
    } else if (m_algoChoice == 3) {
        pss.str(""); pss << std::setprecision(1) << m_saParams.TI;
        drawRow("T-Initial:", pss.str(), y); y += 15.f;
        pss.str(""); pss << std::setprecision(4) << m_saParams.TF;
        drawRow("T-Final:", pss.str(), y); y += 15.f;
        pss.str(""); pss << std::setprecision(4) << m_saParams.FR;
        drawRow("Cool. Rate:", pss.str(), y); y += 15.f;
    } else if (m_algoChoice == 4) {
        drawRow("Pop. Size:", std::to_string(m_gaParams.popSize), y); y += 14.f;
        drawRow("Generations:", std::to_string(m_gaParams.generations), y); y += 14.f;
        pss.str(""); pss << std::setprecision(1) << m_gaParams.mutationRate*100.0 << "%";
        drawRow("Mut. Rate:", pss.str(), y); y += 14.f;
        drawRow("Tournament:", std::to_string(m_gaParams.tournamentSize), y); y += 14.f;
        pss.str(""); pss << std::setprecision(1) << m_gaParams.gi*100.0 << "%";
        drawRow("Elite:", pss.str(), y); y += 14.f;
        pss.str(""); pss << std::setprecision(1) << m_gaParams.br*100.0 << "%";
        drawRow("Breed:", pss.str(), y); y += 14.f;
        drawRow("Stagnation:", std::to_string(m_gaParams.stagnation), y); y += 14.f;
    }

    y += 4.f;
    drawDivider(y); y += 10.f;

    // Per-run results (scrollable)
    drawSecHdr("PER-RUN RESULTS  (" + std::to_string(m_numRuns) + " runs, scroll)", y); y += 18.f;

    float runsAreaY0 = y;
    constexpr float SUMMARY_H = 100.f;
    constexpr float DIV2_H    = 12.f;
    float runsAreaY1 = STATS_PY + STATS_PH - SUMMARY_H - DIV2_H - 8.f;
    float runsAreaH  = runsAreaY1 - runsAreaY0;
    if (runsAreaH < 20.f) runsAreaH = 20.f;

    constexpr float ROW_H = 16.f;
    int visRows = std::max(1, (int)(runsAreaH / ROW_H));
    int maxScroll = std::max(0, m_numRuns - visRows);
    if (m_statsScroll < 0) m_statsScroll = 0;
    if (m_statsScroll > maxScroll) m_statsScroll = maxScroll;

    {
        sf::View rv;
        rv.setViewport(sf::FloatRect(STATS_PX/WIN_W, runsAreaY0/WIN_H,
                                     STATS_PW/WIN_W, runsAreaH/WIN_H));
        rv.setSize(STATS_PW, runsAreaH);
        rv.setCenter(STATS_PW/2.f, runsAreaH/2.f);
        window.setView(rv);

        // Column headers
        {
            sf::Text hdr;
            hdr.setFont(m_assets.font());
            hdr.setCharacterSize(10);
            hdr.setFillColor(sf::Color(140, 125, 90));
            hdr.setString("#   Initial        Final          Gain");
            hdr.setPosition(6.f, 0.f);
            window.draw(hdr);
        }

        for (int i = 0; i < visRows && (m_statsScroll + i) < m_numRuns; i++) {
            int ri = m_statsScroll + i;
            float rowY = (i + 1) * ROW_H;
            double Si = m_initialCosts[ri];
            double Sf = m_finalCosts[ri];
            double G  = (Si > 0) ? 100.0 * std::abs(Si - Sf) / Si : 0.0;

            std::ostringstream ss;
            ss << std::fixed << std::setprecision(1);
            ss << (ri+1);
            sf::Text num;
            num.setFont(m_assets.font()); num.setCharacterSize(11);
            num.setFillColor(sf::Color(160, 150, 120));
            num.setString(ss.str());
            num.setPosition(6.f, rowY + 1.f);
            window.draw(num);

            ss.str(""); ss << Si;
            sf::Text init;
            init.setFont(m_assets.font()); init.setCharacterSize(11);
            init.setFillColor(sf::Color(190, 180, 150));
            init.setString(ss.str());
            init.setPosition(36.f, rowY + 1.f);
            window.draw(init);

            ss.str(""); ss << Sf;
            sf::Text fin;
            fin.setFont(m_assets.font()); fin.setCharacterSize(11);
            fin.setFillColor(sf::Color(190, 180, 150));
            fin.setString(ss.str());
            fin.setPosition(112.f, rowY + 1.f);
            window.draw(fin);

            ss.str(""); ss << std::setprecision(1) << G << "%";
            sf::Text gain;
            gain.setFont(m_assets.font()); gain.setCharacterSize(11);
            gain.setFillColor(Sf < Si - 0.001 ? sf::Color(120, 210, 100) : sf::Color(180, 170, 140));
            gain.setString(ss.str());
            gain.setPosition(200.f, rowY + 1.f);
            window.draw(gain);
        }

        window.setView(window.getDefaultView());
    }

    // Summary / Median results
    drawDivider(runsAreaY1 + 4.f);
    float sy = runsAreaY1 + DIV2_H + 4.f;

    drawSecHdr("MEDIAN SUMMARY", sy); sy += 18.f;

    std::ostringstream rss;
    rss << std::fixed << std::setprecision(1);

    rss.str(""); rss << m_medianInitial;
    drawRow("Median Initial:", rss.str(), sy); sy += 18.f;

    rss.str(""); rss << m_medianFinal;
    drawRow("Median Final:", rss.str(), sy); sy += 18.f;

    {
        rss.str(""); rss << std::setprecision(2) << m_medianGain << "%";
        auto lt = makeText("Gain (G):", 13, sf::Color(160, 145, 110));
        lt.setPosition(STATS_PX + 10.f, sy);
        window.draw(lt);
        bool imp = m_medianFinal < m_medianInitial - 0.001;
        auto vt = makeText(rss.str(), 13, imp ? sf::Color(120, 230, 100) : sf::Color(200, 190, 160), imp);
        vt.setPosition(STATS_PX + 158.f, sy);
        window.draw(vt);
        sy += 18.f;
    }

    // Min / Max final cost
    if (!m_finalCosts.empty()) {
        double minF = *std::min_element(m_finalCosts.begin(), m_finalCosts.end());
        double maxF = *std::max_element(m_finalCosts.begin(), m_finalCosts.end());
        rss.str(""); rss << std::setprecision(1) << minF;
        drawRow("Best run:", rss.str(), sy); sy += 16.f;
        rss.str(""); rss << maxF;
        drawRow("Worst run:", rss.str(), sy);
    }
}

void BenchmarkScreen::draw(sf::RenderWindow& window) {
    window.setView(m_mapView);
    window.draw(m_mapSprite);
    drawEdges(window);

    if (!m_allDone && !m_runner.frames().empty()) {
        const auto& frame = m_runner.frames()[m_frameIdx];
        bool isDone = (m_frameIdx + 1 >= m_runner.frames().size());
        sf::Color pathCol = isDone ? COL_FINAL_PATH : COL_ANIM_PATH;
        float thick = isDone ? 5.f : 3.5f;
        drawPath(window, frame.route, pathCol, thick);
        drawNodes(window, frame.route, pathCol, isDone);
    } else if (m_allDone) {
        drawPath(window, m_runner.bestRoute(), COL_FINAL_PATH, 5.f);
        drawNodes(window, m_runner.bestRoute(), COL_FINAL_PATH, true);
    }

    window.setView(window.getDefaultView());
    if (!m_allDone) drawProgress(window);
    if (m_allDone)  drawFinalStats(window);
    window.draw(m_backBtn);
    window.draw(m_zoomHint);
}
