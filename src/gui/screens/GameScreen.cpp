#include "gui/screens/GameScreen.h"
#include "core/Node.h"
#include "core/Edge.h"
#include <SFML/Window/Keyboard.hpp>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

static constexpr float WIN_W  = 1600.f;
static constexpr float WIN_H  = 900.f;
static constexpr float IMG_W  = 1672.f;
static constexpr float IMG_H  = 941.f;
static constexpr float NODE_R = 8.f;
static constexpr float FRAME_DURATION = 0.05f;

// Stats panel geometry (right side)
static constexpr float STATS_PW = 380.f;
static constexpr float STATS_PH = WIN_H - 40.f;   // 860
static constexpr float STATS_PX = WIN_W - STATS_PW - 10.f;  // 1210
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
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len < 0.5f) return;
    dir /= len;
    sf::Vector2f perp(-dir.y * thickness * 0.5f, dir.x * thickness * 0.5f);
    sf::VertexArray quad(sf::Quads, 4);
    quad[0].position = a + perp;
    quad[1].position = b + perp;
    quad[2].position = b - perp;
    quad[3].position = a - perp;
    quad[0].color = quad[1].color = quad[2].color = quad[3].color = color;
    window.draw(quad);
}

GameScreen::GameScreen(AssetManager& assets, const Graph& graph, const TSP& tsp,
                       NodePositions positions, int algoChoice, int hctTMax, int fixedStart,
                       SAParams saParams, GAParams gaParams)
    : m_assets(assets), m_graph(graph), m_positions(std::move(positions)),
      m_algoChoice(algoChoice), m_hctTMax(hctTMax), m_fixedStart(fixedStart),
      m_saParams(saParams), m_gaParams(gaParams)
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
    m_runner.run(algoChoice, hctTMax, tsp, m_graph.size(), fixedStart, 250, saParams, gaParams);
}

void GameScreen::handleEvent(const sf::Event& event, AppState& next, TransitionData& data) {
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
        next = AppState::MainMenu;

    if (event.type == sf::Event::MouseButtonPressed) {
        sf::Vector2f click(event.mouseButton.x, event.mouseButton.y);
        if (event.mouseButton.button == sf::Mouse::Left) {
            if (m_backBtn.getGlobalBounds().contains(click))
                next = AppState::MainMenu;
        }
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
            sf::Vector2f viewSize = m_mapView.getSize();
            sf::Vector2f delta;
            delta.x = (float)(m_panStart.x - curr.x) * (viewSize.x / WIN_W);
            delta.y = (float)(m_panStart.y - curr.y) * (viewSize.y / WIN_H);
            m_mapView.move(delta);
            m_panStart = curr;
        }
    }

    if (event.type == sf::Event::MouseWheelScrolled) {
        float mx = event.mouseWheelScroll.x;
        float my = event.mouseWheelScroll.y;
        bool overPanel = m_animDone &&
            mx >= STATS_PX && mx <= STATS_PX + STATS_PW &&
            my >= STATS_PY && my <= STATS_PY + STATS_PH;

        if (overPanel) {
            m_statsScroll -= (int)event.mouseWheelScroll.delta;
            if (m_statsScroll < 0) m_statsScroll = 0;
        } else {
            float factor = (event.mouseWheelScroll.delta > 0) ? 0.85f : 1.f / 0.85f;
            float newZoom = m_currentZoom * factor;
            if (newZoom >= 0.1f && newZoom <= 3.f) {
                m_currentZoom = newZoom;
                sf::Vector2f before = m_mapView.getCenter()
                    + sf::Vector2f(
                        (mx - WIN_W * 0.5f) * m_mapView.getSize().x / WIN_W,
                        (my - WIN_H * 0.5f) * m_mapView.getSize().y / WIN_H);
                m_mapView.zoom(factor);
                sf::Vector2f after = m_mapView.getCenter()
                    + sf::Vector2f(
                        (mx - WIN_W * 0.5f) * m_mapView.getSize().x / WIN_W,
                        (my - WIN_H * 0.5f) * m_mapView.getSize().y / WIN_H);
                m_mapView.move(before - after);
            }
        }
    }

    (void)data;
}

void GameScreen::update(float dt) {
    if (m_animDone) return;
    if (m_runner.frames().empty()) { m_animDone = true; return; }
    m_animTimer += dt;
    if (m_animTimer >= FRAME_DURATION) {
        m_animTimer -= FRAME_DURATION;
        if (m_frameIdx + 1 < m_runner.frames().size())
            m_frameIdx++;
        else
            m_animDone = true;
    }
}

void GameScreen::drawEdges(sf::RenderWindow& window) {
    int n = m_graph.size();
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (!m_graph.hasEdge(i, j)) continue;
            if (!m_positions.isPlaced(i) || !m_positions.isPlaced(j)) continue;
            drawThickLine(window, m_positions.get(i), m_positions.get(j),
                          COL_EDGE_ROAD, 2.0f);
        }
    }
}

void GameScreen::drawPath(sf::RenderWindow& window, const std::vector<int>& route,
                          sf::Color color, float thickness) {
    if (route.empty()) return;
    int n = (int)route.size();
    for (int i = 0; i < n; i++) {
        int a = route[i], b = route[(i + 1) % n];
        if (!m_positions.isPlaced(a) || !m_positions.isPlaced(b)) continue;
        drawThickLine(window, m_positions.get(a), m_positions.get(b), color, thickness);
    }
}

void GameScreen::drawNodes(sf::RenderWindow& window, const std::vector<int>& highlight,
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
        if (!highlight.empty())
            for (int h : highlight) if (h == i) { inPath = true; break; }

        bool isStart = (i == startNode);
        float r = isStart ? NODE_R * 1.8f : NODE_R;
        circle.setRadius(r);
        circle.setOrigin(r, r);
        circle.setPosition(pos);
        circle.setOutlineThickness(isStart ? 3.f : 2.f);
        circle.setOutlineColor(isStart ? sf::Color(255, 255, 255, 240) : COL_NODE_OUT);
        circle.setFillColor(isStart ? sf::Color(255, 220, 60) :
                            (inPath && showOrder) ? pathColor : COL_NODE_FILL);
        window.draw(circle);

        if (showOrder && visitOrder[i] >= 0) {
            label.setString(std::to_string(visitOrder[i]));
            label.setFillColor(sf::Color(255, 255, 255, 230));
            label.setPosition(pos.x + r + 1.f, pos.y - 10.f);
            window.draw(label);
        }

        if (isStart) {
            label.setString("START");
            label.setFillColor(sf::Color(255, 240, 100, 240));
            label.setPosition(pos.x + r + 2.f, pos.y - 20.f);
            window.draw(label);
        }

        label.setString(m_graph.getNode(i).name.substr(0, 14));
        label.setFillColor(sf::Color(240, 230, 200, 170));
        label.setPosition(pos.x + r + 1.f, pos.y + 2.f);
        window.draw(label);
    }

    circle.setRadius(NODE_R);
    circle.setOrigin(NODE_R, NODE_R);
}

void GameScreen::drawStats(sf::RenderWindow& window) {
    // Panel background
    sf::RectangleShape panel(sf::Vector2f(STATS_PW, STATS_PH));
    panel.setPosition(STATS_PX, STATS_PY);
    panel.setFillColor(sf::Color(10, 8, 6, 218));
    panel.setOutlineColor(sf::Color(150, 120, 60));
    panel.setOutlineThickness(1.5f);
    window.draw(panel);

    // --- Helpers ---
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
        sf::RectangleShape div(sf::Vector2f(STATS_PW - 16.f, 1.f));
        div.setPosition(STATS_PX + 8.f, y);
        div.setFillColor(sf::Color(100, 80, 40, 180));
        window.draw(div);
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

    // === Title ===
    {
        auto title = makeText("Algorithm Results", 15, sf::Color(220, 200, 150), true);
        auto tb = title.getLocalBounds();
        title.setOrigin(tb.left + tb.width / 2.f, tb.top);
        title.setPosition(STATS_PX + STATS_PW / 2.f, y);
        window.draw(title);
        y += 26.f;
    }
    drawDivider(y); y += 10.f;

    // === Configuration ===
    drawSecHdr("CONFIGURATION", y); y += 18.f;

    static const char* ALGO_NAMES[] = {
        "", "Hill Climbing", "HC with Tries", "Simulated Annealing", "Genetic Algorithm"
    };
    drawRow("Algorithm:", ALGO_NAMES[m_algoChoice], y); y += 18.f;
    drawRow("Nodes:", std::to_string(m_graph.size()), y); y += 18.f;

    if (m_fixedStart >= 0) {
        std::string sname = m_graph.getNode(m_fixedStart).name;
        if ((int)sname.size() > 16) sname = sname.substr(0, 15) + ".";
        drawRow("Fixed Start:", sname, y);
    } else {
        drawRow("Fixed Start:", "None", y);
    }
    y += 18.f;

    std::ostringstream pss;
    pss << std::fixed;
    if (m_algoChoice == 2) {
        drawRow("Max Tries:", std::to_string(m_hctTMax), y); y += 16.f;
    } else if (m_algoChoice == 3) {
        pss.str(""); pss << std::setprecision(1) << m_saParams.TI;
        drawRow("T-Initial:", pss.str(), y); y += 16.f;
        pss.str(""); pss << std::setprecision(4) << m_saParams.TF;
        drawRow("T-Final:", pss.str(), y); y += 16.f;
        pss.str(""); pss << std::setprecision(4) << m_saParams.FR;
        drawRow("Cool. Rate:", pss.str(), y); y += 16.f;
    } else if (m_algoChoice == 4) {
        drawRow("Pop. Size:", std::to_string(m_gaParams.popSize), y); y += 15.f;
        drawRow("Generations:", std::to_string(m_gaParams.generations), y); y += 15.f;
        pss.str(""); pss << std::setprecision(1) << m_gaParams.mutationRate * 100.0 << "%";
        drawRow("Mut. Rate:", pss.str(), y); y += 15.f;
        drawRow("Tournament:", std::to_string(m_gaParams.tournamentSize), y); y += 15.f;
        pss.str(""); pss << std::setprecision(1) << m_gaParams.gi * 100.0 << "%";
        drawRow("Elite:", pss.str(), y); y += 15.f;
        pss.str(""); pss << std::setprecision(1) << m_gaParams.br * 100.0 << "%";
        drawRow("Breed:", pss.str(), y); y += 15.f;
        drawRow("Stagnation:", std::to_string(m_gaParams.stagnation), y); y += 15.f;
    }

    y += 4.f;
    drawDivider(y); y += 10.f;

    // === Route Edges (scrollable) ===
    const auto& route = m_runner.bestRoute();
    int numEdges = (int)route.size();
    {
        std::string hdr = "ROUTE  (" + std::to_string(numEdges) + " steps, scroll to see all)";
        drawSecHdr(hdr, y); y += 18.f;
    }

    float edgeAreaY0 = y;
    constexpr float RESULTS_H = 80.f;
    constexpr float DIV2_H    = 12.f;
    float edgeAreaY1 = STATS_PY + STATS_PH - RESULTS_H - DIV2_H - 8.f;
    float edgeAreaH  = edgeAreaY1 - edgeAreaY0;
    if (edgeAreaH < 20.f) edgeAreaH = 20.f;

    constexpr float ROW_H = 17.f;
    int visRows = std::max(1, (int)(edgeAreaH / ROW_H));

    int maxScroll = std::max(0, numEdges - visRows);
    if (m_statsScroll < 0) m_statsScroll = 0;
    if (m_statsScroll > maxScroll) m_statsScroll = maxScroll;

    // Determine which edge the mouse is hovering over
    int hoveredEdge = -1;
    if (m_mousePos.x >= STATS_PX && m_mousePos.x <= STATS_PX + STATS_PW &&
        m_mousePos.y >= edgeAreaY0 && m_mousePos.y < edgeAreaY1) {
        int rel = (int)((m_mousePos.y - edgeAreaY0) / ROW_H);
        int abs_i = m_statsScroll + rel;
        if (abs_i >= 0 && abs_i < numEdges) hoveredEdge = abs_i;
    }

    // Draw rows clipped to the edge-list viewport
    {
        sf::View edgeView;
        edgeView.setViewport(sf::FloatRect(
            STATS_PX / WIN_W, edgeAreaY0 / WIN_H,
            STATS_PW / WIN_W, edgeAreaH / WIN_H));
        edgeView.setSize(STATS_PW, edgeAreaH);
        edgeView.setCenter(STATS_PW / 2.f, edgeAreaH / 2.f);
        window.setView(edgeView);

        for (int i = 0; i < visRows && (m_statsScroll + i) < numEdges; i++) {
            int ei   = m_statsScroll + i;
            int a    = route[ei];
            int b    = route[(ei + 1) % numEdges];
            double c = m_distMatrix.empty() ? 0.0 : m_distMatrix[a][b];
            float rowY = i * ROW_H;
            bool  hov  = (ei == hoveredEdge);

            if (hov) {
                sf::RectangleShape bg(sf::Vector2f(STATS_PW - 4.f, ROW_H - 1.f));
                bg.setPosition(2.f, rowY);
                bg.setFillColor(sf::Color(45, 35, 14, 200));
                window.draw(bg);
            }

            std::string an = m_graph.getNode(a).name;
            std::string bn = m_graph.getNode(b).name;
            if ((int)an.size() > 11) an = an.substr(0, 10) + ".";
            if ((int)bn.size() > 11) bn = bn.substr(0, 10) + ".";

            sf::Text rowTxt;
            rowTxt.setFont(m_assets.font());
            rowTxt.setCharacterSize(11);
            rowTxt.setFillColor(hov ? sf::Color(255, 240, 160) : sf::Color(200, 190, 160));
            rowTxt.setString(std::to_string(ei + 1) + ". " + an + " -> " + bn);
            rowTxt.setPosition(6.f, rowY + 2.f);
            window.draw(rowTxt);

            std::ostringstream cs;
            cs << std::fixed << std::setprecision(1) << c;
            sf::Text costTxt;
            costTxt.setFont(m_assets.font());
            costTxt.setCharacterSize(11);
            costTxt.setFillColor(hov ? sf::Color(255, 235, 110) : sf::Color(220, 205, 150));
            costTxt.setString(cs.str());
            auto cb2 = costTxt.getLocalBounds();
            costTxt.setPosition(STATS_PW - cb2.width - cb2.left - 8.f, rowY + 2.f);
            window.draw(costTxt);
        }

        window.setView(window.getDefaultView());
    }

    // Tooltip for hovered edge (drawn in screen space after restoring default view)
    if (hoveredEdge >= 0 && hoveredEdge < numEdges) {
        int a = route[hoveredEdge];
        int b = route[(hoveredEdge + 1) % numEdges];
        bool direct = m_graph.hasEdge(a, b);
        float tooltipH = direct ? 72.f : 38.f;

        float rowScreenY = edgeAreaY0 + (hoveredEdge - m_statsScroll) * ROW_H;
        float tooltipY   = rowScreenY + ROW_H + 2.f;
        if (tooltipY + tooltipH > STATS_PY + STATS_PH - 8.f)
            tooltipY = rowScreenY - tooltipH - 2.f;
        tooltipY = std::max(tooltipY, edgeAreaY0);

        const float TW = STATS_PW - 20.f;
        sf::RectangleShape tip(sf::Vector2f(TW, tooltipH));
        tip.setPosition(STATS_PX + 10.f, tooltipY);
        tip.setFillColor(sf::Color(22, 17, 8, 245));
        tip.setOutlineColor(sf::Color(160, 130, 60, 200));
        tip.setOutlineThickness(1.f);
        window.draw(tip);

        float ty = tooltipY + 5.f;
        auto drawTipLine = [&](const std::string& s, sf::Color col, bool bold = false) {
            auto t = makeText(s, 11, col, bold);
            t.setPosition(STATS_PX + 14.f, ty);
            window.draw(t);
            ty += 14.f;
        };

        std::string an = m_graph.getNode(a).name;
        std::string bn = m_graph.getNode(b).name;
        drawTipLine(an + " -> " + bn, sf::Color(240, 220, 160), true);

        if (direct) {
            const Edge& e = m_graph.getEdge(a, b);
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(2);
            ss.str(""); ss << "Distance:    " << e.distance;
            drawTipLine(ss.str(), sf::Color(185, 170, 135));
            ss.str(""); ss << "Danger:      " << e.danger << "  (x4)";
            drawTipLine(ss.str(), sf::Color(185, 170, 135));
            ss.str(""); ss << "Difficulty:  " << e.difficulty << "  (x3)";
            drawTipLine(ss.str(), sf::Color(185, 170, 135));
            ss.str(""); ss << "Total:       " << e.weight();
            drawTipLine(ss.str(), sf::Color(220, 200, 150), true);
        } else {
            drawTipLine("Indirect path (no direct edge)", sf::Color(150, 140, 110));
        }
    }

    // === Results (fixed at bottom) ===
    drawDivider(edgeAreaY1 + 4.f);
    float ry = edgeAreaY1 + DIV2_H + 4.f;

    drawSecHdr("RESULTS", ry); ry += 18.f;

    double Si = m_runner.initialCost();
    double Sf = m_runner.bestCost();
    double G  = (Si > 0.0) ? 100.0 * std::abs(Si - Sf) / Si : 0.0;

    std::ostringstream rss;
    rss << std::fixed << std::setprecision(1);

    rss.str(""); rss << Si;
    drawRow("Initial Cost:", rss.str(), ry); ry += 18.f;

    rss.str(""); rss << Sf;
    drawRow("Final Cost:", rss.str(), ry); ry += 18.f;

    {
        rss.str(""); rss << std::setprecision(2) << G << "%";
        auto lt = makeText("Gain (G):", 13, sf::Color(160, 145, 110));
        lt.setPosition(STATS_PX + 10.f, ry);
        window.draw(lt);
        bool improved = Sf < Si - 0.001;
        sf::Color gc = improved ? sf::Color(120, 230, 100) : sf::Color(200, 190, 160);
        auto vt = makeText(rss.str(), 13, gc, improved);
        vt.setPosition(STATS_PX + 158.f, ry);
        window.draw(vt);
    }
}

void GameScreen::draw(sf::RenderWindow& window) {
    // Map content drawn in world-space view
    window.setView(m_mapView);
    window.draw(m_mapSprite);
    drawEdges(window);

    if (!m_animDone && !m_runner.frames().empty()) {
        const auto& frame = m_runner.frames()[m_frameIdx];
        drawPath(window, frame.route, COL_ANIM_PATH, 3.5f);
        drawNodes(window, frame.route, COL_ANIM_PATH, false);
    } else {
        drawPath(window, m_runner.bestRoute(), COL_FINAL_PATH, 5.f);
        drawNodes(window, m_runner.bestRoute(), COL_FINAL_PATH, true);
    }

    // UI drawn in pixel-space
    window.setView(window.getDefaultView());
    if (m_animDone) drawStats(window);
    window.draw(m_backBtn);
    window.draw(m_zoomHint);
}
