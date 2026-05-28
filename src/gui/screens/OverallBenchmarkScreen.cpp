#include "gui/screens/OverallBenchmarkScreen.h"
#include "core/Node.h"
#include "core/Edge.h"
#include <SFML/Window/Keyboard.hpp>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <numeric>

static constexpr float WIN_W  = 1600.f;
static constexpr float WIN_H  = 900.f;
static constexpr float IMG_W  = 1672.f;
static constexpr float IMG_H  = 941.f;
static constexpr float NODE_R = 8.f;
static constexpr float FRAME_DURATION = 0.05f;

static constexpr float STATS_PW = 420.f;
static constexpr float STATS_PH = WIN_H - 40.f;
static constexpr float STATS_PX = WIN_W - STATS_PW - 10.f;
static constexpr float STATS_PY = 20.f;

static const sf::Color COL_EDGE_ROAD  = sf::Color(190, 150,  80, 160);
static const sf::Color COL_ANIM_PATH  = sf::Color(230,  50,  50, 220);
static const sf::Color COL_FINAL_PATH = sf::Color(245, 215,  40, 240);
static const sf::Color COL_NODE_FILL  = sf::Color(230, 220, 190);
static const sf::Color COL_NODE_OUT   = sf::Color( 40,  25,  10);

static const char* ALGO_SHORT[] = {"Hill Climbing", "HC with Tries", "Simul.Annealing", "Genetic Alg."};
static const sf::Color ALGO_COLS[] = {
    sf::Color( 80, 160, 240),
    sf::Color(160,  80, 240),
    sf::Color(240, 160,  40),
    sf::Color( 80, 220, 120)
};

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

double OverallBenchmarkScreen::medianOf(std::vector<double> v) {
    if (v.empty()) return 0.0;
    std::sort(v.begin(), v.end());
    int n = (int)v.size();
    return (n % 2 == 0) ? (v[n/2-1] + v[n/2]) * 0.5 : v[n/2];
}

std::string OverallBenchmarkScreen::routeToStr(const std::vector<int>& r) {
    std::ostringstream ss;
    for (int i = 0; i < (int)r.size(); i++) {
        if (i) ss << ';';
        ss << r[i];
    }
    return ss.str();
}

std::string OverallBenchmarkScreen::costsToStr(const std::vector<double>& v) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2);
    for (int i = 0; i < (int)v.size(); i++) {
        if (i) ss << ';';
        ss << v[i];
    }
    return ss.str();
}

std::string OverallBenchmarkScreen::configStr(int algoChoice, int hctTMax,
                                              const SAParams& sa, const GAParams& ga) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4);
    switch (algoChoice) {
        case 1: return "-";
        case 2: return "t_max=" + std::to_string(hctTMax);
        case 3:
            ss << "TI=" << sa.TI << ",TF=" << sa.TF << ",FR=" << sa.FR;
            return ss.str();
        case 4:
            ss << std::setprecision(1)
               << "pop=" << ga.popSize << ",gen=" << ga.generations
               << ",mut=" << ga.mutationRate*100 << "%"
               << ",tour=" << ga.tournamentSize
               << ",elite=" << ga.gi*100 << "%"
               << ",breed=" << ga.br*100 << "%"
               << ",stag=" << ga.stagnation;
            return ss.str();
        default: return "-";
    }
}

OverallBenchmarkScreen::OverallBenchmarkScreen(AssetManager& assets, const Graph& graph,
                                               const TSP& tsp, NodePositions positions,
                                               int hctTMax, int fixedStart,
                                               SAParams saParams, GAParams gaParams,
                                               int numRuns, const std::string& projectRoot)
    : m_assets(assets), m_graph(graph), m_positions(std::move(positions)), m_tsp(tsp),
      m_hctTMax(hctTMax), m_fixedStart(fixedStart),
      m_saParams(saParams), m_gaParams(gaParams),
      m_numRuns(std::max(1, numRuns)), m_projectRoot(projectRoot)
{
    m_mapSprite.setTexture(m_assets.mapTexture());
    m_mapView = sf::View(sf::FloatRect(0.f, 0.f, IMG_W, IMG_H));

    m_backBtn.setFont(m_assets.font());
    m_backBtn.setString("[ Back ]");
    m_backBtn.setCharacterSize(18);
    m_backBtn.setFillColor(sf::Color(200, 180, 120));
    m_backBtn.setPosition(12.f, WIN_H - 30.f);

    if (m_graph.size() == NodePositions::NODE_COUNT && !m_positions.allPlaced())
        m_positions.applyFallback();

    // Initialise result slots
    for (int i = 0; i < 4; i++) {
        m_results[i].algoChoice = i + 1;
        m_results[i].initialCosts.reserve(m_numRuns);
        m_results[i].finalCosts.reserve(m_numRuns);
    }

    startRun();
}

void OverallBenchmarkScreen::startRun() {
    int algoChoice = m_algoIdx + 1;
    m_runner.run(algoChoice, m_hctTMax, m_tsp, m_graph.size(),
                 m_fixedStart, 250, m_saParams, m_gaParams);
    m_runIdx++;
    m_frameIdx  = 0;
    m_animTimer = 0.f;
    m_animDone  = false;

    auto& res = m_results[m_algoIdx];
    res.initialCosts.push_back(m_runner.initialCost());
    res.finalCosts.push_back(m_runner.bestCost());

    // Track best run (minimum final cost)
    if (res.finalCosts.back() <= *std::min_element(res.finalCosts.begin(), res.finalCosts.end())) {
        res.bestInitRoute  = m_runner.initialRoute();
        res.bestFinalRoute = m_runner.bestRoute();
    }
}

void OverallBenchmarkScreen::finalizeAlgo() {
    auto& res = m_results[m_algoIdx];
    res.medianInitial = medianOf(res.initialCosts);
    res.medianFinal   = medianOf(res.finalCosts);
    res.gain = (res.medianInitial > 0.0)
        ? 100.0 * std::abs(res.medianInitial - res.medianFinal) / res.medianInitial
        : 0.0;
    // Ensure bestFinalRoute is set (it should already be from startRun)
    if (res.bestFinalRoute.empty()) res.bestFinalRoute = m_runner.bestRoute();
    if (res.bestInitRoute.empty())  res.bestInitRoute  = m_runner.initialRoute();
}

void OverallBenchmarkScreen::writeCSV() {
    // Build timestamp
    std::time_t now = std::time(nullptr);
    std::tm* ltm = std::localtime(&now);
    char ts[32], ts2[32];
    std::strftime(ts,  sizeof(ts),  "%Y%m%d_%H%M%S", ltm);
    std::strftime(ts2, sizeof(ts2), "%Y-%m-%d %H:%M:%S", ltm);

    m_csvPath = m_projectRoot + "/results/overall_" + ts + ".csv";
    std::ofstream f(m_csvPath);
    if (!f.is_open()) return;

    f << "# Overall Benchmark -- " << ts2
      << "  nodes=" << m_graph.size()
      << "  runs_per_algo=" << m_numRuns << "\n";

    f << "algorithm,config,runs,median_initial,median_final,gain_pct,"
      << "best_run_final,worst_run_final,"
      << "per_run_initial_costs,per_run_final_costs,"
      << "initial_route,best_route\n";

    f << std::fixed << std::setprecision(4);

    for (int i = 0; i < 4; i++) {
        auto& res = m_results[i];
        double bestF  = *std::min_element(res.finalCosts.begin(), res.finalCosts.end());
        double worstF = *std::max_element(res.finalCosts.begin(), res.finalCosts.end());

        static const char* ALGO_FULL[] = {"Hill Climbing","HC with Tries","Simulated Annealing","Genetic Algorithm"};
        std::string cfg = configStr(res.algoChoice, m_hctTMax, m_saParams, m_gaParams);

        f << '"' << ALGO_FULL[i] << '"' << ','
          << '"' << cfg << '"' << ','
          << m_numRuns << ','
          << res.medianInitial << ','
          << res.medianFinal << ','
          << res.gain << ','
          << bestF << ',' << worstF << ','
          << '"' << costsToStr(res.initialCosts) << '"' << ','
          << '"' << costsToStr(res.finalCosts)   << '"' << ','
          << '"' << routeToStr(res.bestInitRoute)  << '"' << ','
          << '"' << routeToStr(res.bestFinalRoute) << '"' << '\n';
    }
}

void OverallBenchmarkScreen::handleEvent(const sf::Event& event, AppState& next, TransitionData& data) {
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

void OverallBenchmarkScreen::update(float dt) {
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
        if (m_runIdx < m_numRuns) {
            startRun();
        } else {
            finalizeAlgo();
            if (m_algoIdx < 3) {
                m_algoIdx++;
                m_runIdx = 0;
                startRun();
            } else {
                writeCSV();
                m_allDone = true;
            }
        }
    }
}

void OverallBenchmarkScreen::drawEdges(sf::RenderWindow& window) {
    int n = m_graph.size();
    for (int i = 0; i < n-1; i++)
        for (int j = i+1; j < n; j++) {
            if (!m_graph.hasEdge(i,j)) continue;
            if (!m_positions.isPlaced(i) || !m_positions.isPlaced(j)) continue;
            drawThickLine(window, m_positions.get(i), m_positions.get(j), COL_EDGE_ROAD, 2.f);
        }
}

void OverallBenchmarkScreen::drawPath(sf::RenderWindow& window, const std::vector<int>& route,
                                      sf::Color color, float thickness) {
    int n = (int)route.size();
    for (int i = 0; i < n; i++) {
        int a = route[i], b = route[(i+1)%n];
        if (!m_positions.isPlaced(a) || !m_positions.isPlaced(b)) continue;
        drawThickLine(window, m_positions.get(a), m_positions.get(b), color, thickness);
    }
}

void OverallBenchmarkScreen::drawNodes(sf::RenderWindow& window, const std::vector<int>& highlight,
                                       sf::Color pathColor, bool showOrder) {
    int n = m_graph.size();
    sf::CircleShape circle(NODE_R);
    circle.setOrigin(NODE_R, NODE_R);
    circle.setOutlineThickness(2.f);
    circle.setOutlineColor(COL_NODE_OUT);
    sf::Text label;
    label.setFont(m_assets.font());
    label.setCharacterSize(8);
    int startNode = (!highlight.empty() && showOrder) ? highlight[0] : -1;

    std::vector<int> visitOrder(n, -1);
    if (showOrder)
        for (int i = 0; i < (int)highlight.size(); i++)
            visitOrder[highlight[i]] = i + 1;

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
            label.setFillColor(sf::Color(255,255,255,220));
            label.setPosition(pos.x+r+1.f, pos.y-10.f);
            window.draw(label);
        }
        label.setString(m_graph.getNode(i).name.substr(0, 12));
        label.setFillColor(sf::Color(240,230,200,160));
        label.setPosition(pos.x+r+1.f, pos.y+2.f);
        window.draw(label);
    }
}

void OverallBenchmarkScreen::drawProgressPanel(sf::RenderWindow& window) {
    constexpr float PW = 340.f, PH = 210.f;
    float px = STATS_PX - PW - 10.f;
    float py = STATS_PY;

    sf::RectangleShape panel(sf::Vector2f(PW, PH));
    panel.setPosition(px, py);
    panel.setFillColor(sf::Color(10, 8, 6, 215));
    panel.setOutlineColor(sf::Color(150, 120, 60));
    panel.setOutlineThickness(1.5f);
    window.draw(panel);

    auto txt = [&](const std::string& s, unsigned sz, sf::Color c, float x, float y, bool bold = false) {
        sf::Text t;
        t.setFont(m_assets.font());
        t.setCharacterSize(sz);
        t.setFillColor(c);
        if (bold) t.setStyle(sf::Text::Bold);
        t.setString(s);
        t.setPosition(x, y);
        window.draw(t);
    };

    txt("Overall Benchmark Progress", 13, sf::Color(220, 200, 150), px+10.f, py+8.f, true);

    float rowY = py + 28.f;
    for (int i = 0; i < 4; i++) {
        bool done    = (i < m_algoIdx);
        bool current = (i == m_algoIdx);

        sf::Color ic = done ? sf::Color(80,200,80) : current ? ALGO_COLS[i] : sf::Color(80,75,60);

        // Status icon
        std::string icon = done ? "[v]" : current ? "[>]" : "[ ]";
        txt(icon, 11, ic, px+10.f, rowY);

        txt(ALGO_SHORT[i], 12, ic, px+42.f, rowY);

        if (done) {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(1) << "med:" << m_results[i].medianFinal;
            txt(ss.str(), 11, sf::Color(140,200,140), px+170.f, rowY);
        } else if (current) {
            // Run progress bar
            float barX  = px + 170.f;
            float barW  = PW - 180.f;
            float filled = m_numRuns > 0 ? (float)(m_runIdx - 1) / m_numRuns * barW : 0.f;

            sf::RectangleShape bg(sf::Vector2f(barW, 10.f));
            bg.setPosition(barX, rowY + 2.f);
            bg.setFillColor(sf::Color(35, 28, 15, 200));
            bg.setOutlineColor(sf::Color(100,80,40,140));
            bg.setOutlineThickness(1.f);
            window.draw(bg);
            if (filled > 0.f) {
                sf::RectangleShape fill(sf::Vector2f(filled, 10.f));
                fill.setPosition(barX, rowY + 2.f);
                fill.setFillColor(ALGO_COLS[i]);
                window.draw(fill);
            }
            std::ostringstream ss;
            ss << m_runIdx << "/" << m_numRuns;
            txt(ss.str(), 10, sf::Color(180,170,140), barX + barW + 4.f, rowY);
        }
        rowY += 22.f;
    }

    // Total progress bar
    rowY += 4.f;
    sf::RectangleShape div(sf::Vector2f(PW - 20.f, 1.f));
    div.setPosition(px + 10.f, rowY);
    div.setFillColor(sf::Color(100, 80, 40, 160));
    window.draw(div);
    rowY += 8.f;

    int totalRuns = 4 * m_numRuns;
    int doneRuns  = m_algoIdx * m_numRuns + (m_runIdx - 1);
    float barW    = PW - 20.f;
    float filled  = totalRuns > 0 ? (float)doneRuns / totalRuns * barW : 0.f;

    sf::RectangleShape totBg(sf::Vector2f(barW, 12.f));
    totBg.setPosition(px + 10.f, rowY);
    totBg.setFillColor(sf::Color(35, 28, 15, 200));
    totBg.setOutlineColor(sf::Color(100, 80, 40, 140));
    totBg.setOutlineThickness(1.f);
    window.draw(totBg);
    if (filled > 0.f) {
        sf::RectangleShape totFill(sf::Vector2f(filled, 12.f));
        totFill.setPosition(px + 10.f, rowY);
        totFill.setFillColor(sf::Color(80, 160, 80, 220));
        window.draw(totFill);
    }
    std::ostringstream tot_ss;
    tot_ss << "Total: " << doneRuns << " / " << totalRuns << " runs";
    txt(tot_ss.str(), 11, sf::Color(180, 170, 140), px + 10.f, rowY + 16.f);
}

void OverallBenchmarkScreen::drawFinalTable(sf::RenderWindow& window) {
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

    float y = STATS_PY + 8.f;

    // Title
    {
        auto t = makeText("Overall Benchmark Results", 14, sf::Color(220, 200, 150), true);
        auto tb = t.getLocalBounds();
        t.setOrigin(tb.left + tb.width/2.f, tb.top);
        t.setPosition(STATS_PX + STATS_PW/2.f, y);
        window.draw(t);
        y += 24.f;
    }
    drawDivider(y); y += 10.f;

    // Build sorted indices by medianFinal ascending
    int sorted[4] = {0, 1, 2, 3};
    std::sort(std::begin(sorted), std::end(sorted), [&](int a, int b) {
        return m_results[a].medianFinal < m_results[b].medianFinal;
    });
    int bestIdx = sorted[0]; // lowest median final = best

    // Column header
    {
        auto hdr = makeText("Algorithm       Med.Init   Med.Final  Gain", 10, sf::Color(140,125,90));
        hdr.setPosition(STATS_PX + 10.f, y);
        window.draw(hdr);
        y += 14.f;
    }
    drawDivider(y); y += 6.f;

    // Rows
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1);

    for (int rank = 0; rank < 4; rank++) {
        int i = sorted[rank];
        auto& r = m_results[i];
        bool best = (i == bestIdx);

        // Row highlight for best
        if (best) {
            sf::RectangleShape hl(sf::Vector2f(STATS_PW - 12.f, 30.f));
            hl.setPosition(STATS_PX + 6.f, y);
            hl.setFillColor(sf::Color(60, 50, 15, 160));
            window.draw(hl);
        }

        // Star + algo name
        sf::Color nameCol = best ? sf::Color(255, 230, 80) : ALGO_COLS[i];
        std::string star = best ? "* " : "  ";
        auto algo_t = makeText(star + ALGO_SHORT[i], 12, nameCol, best);
        algo_t.setPosition(STATS_PX + 8.f, y + 4.f);
        window.draw(algo_t);

        // Median initial
        ss.str(""); ss << r.medianInitial;
        auto mi_t = makeText(ss.str(), 12, sf::Color(185, 175, 145));
        mi_t.setPosition(STATS_PX + 178.f, y + 4.f);
        window.draw(mi_t);

        // Median final (color-coded by rank)
        ss.str(""); ss << r.medianFinal;
        sf::Color fc = (rank == 0) ? sf::Color(120,230,100) :
                       (rank == 1) ? sf::Color(200,210,120) :
                       (rank == 2) ? sf::Color(210,170,100) :
                                     sf::Color(210,120,100);
        auto mf_t = makeText(ss.str(), 12, fc, best);
        mf_t.setPosition(STATS_PX + 258.f, y + 4.f);
        window.draw(mf_t);

        // Gain
        ss.str(""); ss << std::setprecision(2) << r.gain << "%";
        auto g_t = makeText(ss.str(), 12, fc, false);
        g_t.setPosition(STATS_PX + 348.f, y + 4.f);
        window.draw(g_t);

        y += 32.f;
    }

    drawDivider(y); y += 10.f;

    // Detailed stats for each algo (Best / Worst run)
    auto drawSecHdr = [&](const std::string& s, float ry) {
        auto t = makeText(s, 11, sf::Color(180, 155, 90), true);
        t.setPosition(STATS_PX + 10.f, ry);
        window.draw(t);
    };
    auto drawRow = [&](const std::string& lbl, const std::string& val, float ry,
                       sf::Color vc = sf::Color(230,220,180)) {
        auto lt = makeText(lbl, 12, sf::Color(155, 140, 105));
        lt.setPosition(STATS_PX + 10.f, ry);
        window.draw(lt);
        auto vt = makeText(val, 12, vc);
        vt.setPosition(STATS_PX + 195.f, ry);
        window.draw(vt);
    };

    drawSecHdr("PER-ALGORITHM DETAIL", y); y += 18.f;

    for (int rank = 0; rank < 4; rank++) {
        int i = sorted[rank];
        auto& r = m_results[i];
        bool best = (i == bestIdx);

        double bestRun  = *std::min_element(r.finalCosts.begin(), r.finalCosts.end());
        double worstRun = *std::max_element(r.finalCosts.begin(), r.finalCosts.end());

        std::string prefix = best ? "* " : "  ";
        sf::Color nc = best ? sf::Color(255, 230, 80) : ALGO_COLS[i];

        auto hdr = makeText(prefix + ALGO_SHORT[i], 12, nc, best);
        hdr.setPosition(STATS_PX + 8.f, y);
        window.draw(hdr);
        y += 16.f;

        ss.str(""); ss << std::setprecision(1) << bestRun;
        drawRow("  Best run:", ss.str(), y, sf::Color(120,230,100)); y += 14.f;
        ss.str(""); ss << worstRun;
        drawRow("  Worst run:", ss.str(), y, sf::Color(210,140,120)); y += 14.f;

        // Algo config compact
        std::string cfg = configStr(r.algoChoice, m_hctTMax, m_saParams, m_gaParams);
        if (cfg.size() > 30) cfg = cfg.substr(0, 29) + "..";
        drawRow("  Config:", cfg, y, sf::Color(160,150,120)); y += 14.f;

        y += 4.f;
    }

    drawDivider(y); y += 10.f;

    // CSV path
    if (!m_csvPath.empty()) {
        auto t1 = makeText("Results saved to:", 11, sf::Color(140,130,100));
        t1.setPosition(STATS_PX + 10.f, y);
        window.draw(t1);
        y += 14.f;

        // Show just the filename part
        std::string fname = m_csvPath;
        auto pos = fname.rfind('/');
        if (pos != std::string::npos) fname = "results/" + fname.substr(pos+1);
        auto t2 = makeText(fname, 11, sf::Color(120, 200, 180));
        t2.setPosition(STATS_PX + 10.f, y);
        window.draw(t2);
    }
}

void OverallBenchmarkScreen::draw(sf::RenderWindow& window) {
    window.setView(m_mapView);
    window.draw(m_mapSprite);
    drawEdges(window);

    if (!m_runner.frames().empty()) {
        const auto& frame = m_runner.frames()[m_frameIdx];
        bool isDone = m_animDone || (m_frameIdx + 1 >= m_runner.frames().size());
        sf::Color pathCol = isDone ? COL_FINAL_PATH : ALGO_COLS[m_algoIdx];
        float thick = isDone ? 5.f : 3.5f;
        drawPath(window, frame.route, pathCol, thick);
        drawNodes(window, frame.route, pathCol, isDone && !m_allDone);
    }

    window.setView(window.getDefaultView());

    if (!m_allDone) drawProgressPanel(window);
    if (m_allDone)  drawFinalTable(window);

    window.draw(m_backBtn);

    // Bottom hint
    sf::Text hint;
    hint.setFont(m_assets.font());
    hint.setCharacterSize(13);
    hint.setFillColor(sf::Color(160, 150, 120, 180));
    hint.setString("Scroll: zoom  |  Right-drag: pan  |  ESC: back");
    hint.setPosition(130.f, WIN_H - 26.f);
    window.draw(hint);
}
