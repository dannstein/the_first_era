#include "gui/screens/GameScreen.h"
#include "core/Node.h"
#include <SFML/Window/Keyboard.hpp>
#include <sstream>
#include <iomanip>
#include <cmath>

static constexpr float WIN_W  = 1600.f;
static constexpr float WIN_H  = 900.f;
static constexpr float IMG_W  = 1672.f;
static constexpr float IMG_H  = 941.f;
static constexpr float NODE_R = 8.f;      // node radius in world-space (image pixels)
static constexpr float FRAME_DURATION = 0.05f;

static const sf::Color COL_EDGE_ROAD  = sf::Color(190, 150,  80, 160);
static const sf::Color COL_ANIM_PATH  = sf::Color(230,  50,  50, 220);
static const sf::Color COL_FINAL_PATH = sf::Color(245, 215,  40, 240);
static const sf::Color COL_NODE_FILL  = sf::Color(230, 220, 190);
static const sf::Color COL_NODE_OUT   = sf::Color( 40,  25,  10);

// Draws a thick line in the current view (world-space coordinates).
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
                       NodePositions positions, int algoChoice, int hctTMax)
    : m_assets(assets), m_graph(graph), m_positions(std::move(positions))
{
    // Map sprite drawn at (0,0) with no scaling; the view handles zoom/pan.
    m_mapSprite.setTexture(m_assets.mapTexture());

    // View shows the full image by default, stretched to fill the window.
    m_mapView = sf::View(sf::FloatRect(0.f, 0.f, IMG_W, IMG_H));

    m_backBtn.setFont(m_assets.font());
    m_backBtn.setString("[ Back ]");
    m_backBtn.setCharacterSize(18);
    m_backBtn.setFillColor(sf::Color(200, 180, 120));
    m_backBtn.setPosition(12.f, WIN_H - 30.f);

    m_zoomHint.setFont(m_assets.font());
    m_zoomHint.setCharacterSize(13);
    m_zoomHint.setFillColor(sf::Color(160, 150, 120, 180));
    m_zoomHint.setString("Scroll: zoom  |  Right-drag: pan  |  ESC: back");
    m_zoomHint.setPosition(130.f, WIN_H - 26.f);

    if (!m_positions.allPlaced())
        m_positions.applyFallback();

    m_runner.run(algoChoice, hctTMax, tsp, 50, 250);
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

    if (event.type == sf::Event::MouseMoved && m_panning) {
        sf::Vector2i curr(event.mouseMove.x, event.mouseMove.y);
        // Convert pixel delta to world-space delta via the view transform.
        sf::Vector2f worldA = sf::Vector2f(m_panStart);
        sf::Vector2f worldB = sf::Vector2f(curr);
        // Scale the pixel delta by (view size / window size) to get world units.
        sf::Vector2f viewSize = m_mapView.getSize();
        sf::Vector2f delta;
        delta.x = (worldA.x - worldB.x) * (viewSize.x / WIN_W);
        delta.y = (worldA.y - worldB.y) * (viewSize.y / WIN_H);
        m_mapView.move(delta);
        m_panStart = curr;
    }

    if (event.type == sf::Event::MouseWheelScrolled) {
        float factor = (event.mouseWheelScroll.delta > 0) ? 0.85f : 1.f / 0.85f;
        float newZoom = m_currentZoom * factor;
        if (newZoom < 0.1f || newZoom > 3.f) return;
        m_currentZoom = newZoom;
        // Zoom centered on the mouse cursor position.
        sf::Vector2f before = m_mapView.getCenter()
            + sf::Vector2f(
                (event.mouseWheelScroll.x - WIN_W * 0.5f) * m_mapView.getSize().x / WIN_W,
                (event.mouseWheelScroll.y - WIN_H * 0.5f) * m_mapView.getSize().y / WIN_H);
        m_mapView.zoom(factor);
        sf::Vector2f after = m_mapView.getCenter()
            + sf::Vector2f(
                (event.mouseWheelScroll.x - WIN_W * 0.5f) * m_mapView.getSize().x / WIN_W,
                (event.mouseWheelScroll.y - WIN_H * 0.5f) * m_mapView.getSize().y / WIN_H);
        m_mapView.move(before - after);
    }

    if (event.type == sf::Event::MouseWheelScrolled && m_animDone)
        m_statsScroll -= (int)event.mouseWheelScroll.delta;
    if (m_statsScroll < 0) m_statsScroll = 0;

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

    // Reset circle to default radius for next call
    circle.setRadius(NODE_R);
    circle.setOrigin(NODE_R, NODE_R);
}

void GameScreen::drawStats(sf::RenderWindow& window) {
    constexpr float PW    = 320.f;
    constexpr float PH    = WIN_H - 50.f;
    constexpr float PX    = WIN_W - PW - 10.f;
    constexpr float PY    = 20.f;
    constexpr int   VISH  = 18;
    constexpr float ROW_H = 17.f;

    sf::RectangleShape panel(sf::Vector2f(PW, PH));
    panel.setPosition(PX, PY);
    panel.setFillColor(sf::Color(10, 8, 6, 210));
    panel.setOutlineColor(sf::Color(150, 120, 60));
    panel.setOutlineThickness(1.5f);
    window.draw(panel);

    sf::Text header;
    header.setFont(m_assets.font());
    header.setCharacterSize(15);
    header.setFillColor(sf::Color(220, 200, 150));
    header.setStyle(sf::Text::Bold);

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << m_runner.bestCost();
    header.setString("Best cost: " + ss.str());
    header.setPosition(PX + 8.f, PY + 8.f);
    window.draw(header);

    header.setStyle(sf::Text::Regular);
    header.setCharacterSize(13);
    header.setString("Route order:  (scroll with mouse wheel)");
    header.setPosition(PX + 8.f, PY + 28.f);
    window.draw(header);

    const auto& route = m_runner.bestRoute();
    sf::Text row;
    row.setFont(m_assets.font());
    row.setCharacterSize(12);

    int maxScroll = std::max(0, (int)route.size() - VISH);
    if (m_statsScroll > maxScroll) m_statsScroll = maxScroll;

    for (int i = 0; i < VISH && (m_statsScroll + i) < (int)route.size(); i++) {
        int nodeId = route[m_statsScroll + i];
        std::string name = m_graph.getNode(nodeId).name;
        row.setString(std::to_string(m_statsScroll + i + 1) + ". " + name);
        row.setFillColor(sf::Color(210, 200, 170));
        row.setPosition(PX + 8.f, PY + 50.f + i * ROW_H);
        window.draw(row);
    }
}

void GameScreen::draw(sf::RenderWindow& window) {
    // --- Map content drawn in world-space view ---
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

    // --- UI drawn in pixel-space ---
    window.setView(window.getDefaultView());
    if (m_animDone) drawStats(window);
    window.draw(m_backBtn);
    window.draw(m_zoomHint);
}
