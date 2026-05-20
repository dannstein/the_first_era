#include "gui/screens/PinScreen.h"
#include "core/Node.h"
#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include <filesystem>

static constexpr float WIN_W = 1600.f;
static constexpr float WIN_H = 900.f;

const PinScreen::RegionDef PinScreen::REGIONS[6] = {
    {"REGION I  — Eryndorheim",   0,  7, sf::Color( 70, 110, 180, 210)},
    {"REGION II — Nocthyr Vale",  8, 15, sf::Color( 80,  40, 120, 210)},
    {"REGION III — Aurelion",    16, 25, sf::Color(170, 140,  40, 210)},
    {"REGION IV — Morvhal Mire", 26, 33, sf::Color( 40,  80,  40, 210)},
    {"REGION V — Solkarath",     34, 41, sf::Color(180, 110,  30, 210)},
    {"REGION VI — Drakmord",     42, 49, sf::Color(160,  35,  35, 210)},
};

int PinScreen::rowToNode(int row) {
    if (row < 0) return -1;
    int cursor = 0;
    for (int r = 0; r < 6; r++) {
        if (row == cursor) return -1;
        cursor++;
        int count = REGIONS[r].last - REGIONS[r].first + 1;
        if (row < cursor + count)
            return REGIONS[r].first + (row - cursor);
        cursor += count;
    }
    return -1;
}

PinScreen::PinScreen(AssetManager& assets, const Graph& graph,
                     NodePositions positions, const std::string& configDir)
    : m_assets(assets), m_graph(graph),
      m_positions(std::move(positions)), m_configDir(configDir)
{
    m_mapSprite.setTexture(m_assets.mapTexture());

    // Map view viewport: right of sidebar.
    float vx = SIDEBAR_W / WIN_W;
    float vw = 1.f - vx;
    m_mapView = sf::View(sf::FloatRect(0.f, 0.f, MAP_IMG_W, MAP_IMG_H));
    m_mapView.setViewport(sf::FloatRect(vx, 0.f, vw, 1.f));

    m_saveBtn.setFont(m_assets.font());
    m_saveBtn.setString("[ Save ]");
    m_saveBtn.setCharacterSize(17);
    m_saveBtn.setFillColor(sf::Color(120, 220, 100));

    m_backBtn.setFont(m_assets.font());
    m_backBtn.setString("[ Back ]");
    m_backBtn.setCharacterSize(17);
    m_backBtn.setFillColor(sf::Color(200, 180, 120));

    m_presetLabel.setFont(m_assets.font());
    m_presetLabel.setString("Preset name:");
    m_presetLabel.setCharacterSize(13);
    m_presetLabel.setFillColor(sf::Color(170, 160, 130));

    m_presetInput.setFont(m_assets.font());
    m_presetInput.setCharacterSize(13);
    m_presetInput.setFillColor(sf::Color(255, 240, 180));
}

int PinScreen::nodeNear(sf::Vector2i screenPos, float worldRadius) const {
    if (!m_window) return -1;
    sf::Vector2f worldPos = m_window->mapPixelToCoords(screenPos, m_mapView);
    for (int i = 0; i < NodePositions::NODE_COUNT; i++) {
        if (!m_positions.isPlaced(i)) continue;
        sf::Vector2f p = m_positions.get(i);
        float dx = p.x - worldPos.x, dy = p.y - worldPos.y;
        if (std::sqrt(dx * dx + dy * dy) <= worldRadius) return i;
    }
    return -1;
}

void PinScreen::handleEvent(const sf::Event& event, AppState& next, TransitionData& data) {
    if (event.type == sf::Event::KeyPressed) {
        if (m_editingName) {
            if (event.key.code == sf::Keyboard::Enter)       m_editingName = false;
            else if (event.key.code == sf::Keyboard::BackSpace && !m_presetName.empty())
                m_presetName.pop_back();
            return;
        }
        if (event.key.code == sf::Keyboard::Escape) next = AppState::MainMenu;
    }

    if (event.type == sf::Event::TextEntered && m_editingName) {
        char c = static_cast<char>(event.text.unicode);
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '_' || c == '-')
            if (m_presetName.size() < 20) m_presetName += c;
    }

    if (event.type == sf::Event::MouseWheelScrolled) {
        if (event.mouseWheelScroll.x >= SIDEBAR_W) {
            // Zoom map
            float factor = (event.mouseWheelScroll.delta > 0) ? 0.85f : 1.f / 0.85f;
            float newZoom = m_currentZoom * factor;
            if (newZoom < 0.1f || newZoom > 3.f) return;
            m_currentZoom = newZoom;
            // Zoom centered on mouse cursor
            float mapCenterX = SIDEBAR_W + (WIN_W - SIDEBAR_W) * 0.5f;
            sf::Vector2f before = m_mapView.getCenter()
                + sf::Vector2f(
                    (event.mouseWheelScroll.x - mapCenterX) * m_mapView.getSize().x / (WIN_W - SIDEBAR_W),
                    (event.mouseWheelScroll.y - WIN_H * 0.5f) * m_mapView.getSize().y / WIN_H);
            m_mapView.zoom(factor);
            sf::Vector2f after = m_mapView.getCenter()
                + sf::Vector2f(
                    (event.mouseWheelScroll.x - mapCenterX) * m_mapView.getSize().x / (WIN_W - SIDEBAR_W),
                    (event.mouseWheelScroll.y - WIN_H * 0.5f) * m_mapView.getSize().y / WIN_H);
            m_mapView.move(before - after);
        } else {
            // Scroll sidebar
            m_sideScroll -= (int)event.mouseWheelScroll.delta;
            if (m_sideScroll < 0) m_sideScroll = 0;
            int visRows   = (int)((WIN_H - BOT_AREA) / ITEM_H);
            int maxScroll = std::max(0, TOTAL_ROWS - visRows);
            if (m_sideScroll > maxScroll) m_sideScroll = maxScroll;
        }
    }

    if (event.type == sf::Event::MouseButtonPressed) {
        sf::Vector2f click(event.mouseButton.x, event.mouseButton.y);
        sf::Vector2i clickI(event.mouseButton.x, event.mouseButton.y);

        if (event.mouseButton.button == sf::Mouse::Right && click.x >= SIDEBAR_W) {
            m_panning = true;
            m_panStart = clickI;
            return;
        }

        if (event.mouseButton.button == sf::Mouse::Left) {
            if (m_backBtnRect.contains(click))  { next = AppState::MainMenu; return; }

            if (m_positions.allPlaced() && m_saveBtnRect.contains(click)) {
                std::string path = NodePositions::presetPath(m_configDir, m_presetName);
                std::filesystem::create_directories(m_configDir);
                m_positions.saveToFile(path);
                next = AppState::MainMenu;
                return;
            }

            if (m_presetInputRect.contains(click)) { m_editingName = true; return; }

            // Sidebar: select node
            if (click.x < SIDEBAR_W && click.y < WIN_H - BOT_AREA) {
                int row    = m_sideScroll + (int)(click.y / ITEM_H);
                int nodeId = rowToNode(row);
                if (nodeId >= 0) { m_selected = nodeId; m_editingName = false; }
                return;
            }

            // Map area: place selected node, or re-select a placed one
            if (click.x >= SIDEBAR_W && m_window) {
                sf::Vector2f worldPos = m_window->mapPixelToCoords(clickI, m_mapView);

                if (m_selected >= 0) {
                    // Place the selected node at this world position
                    m_positions.set(m_selected, worldPos);
                    m_selected = -1;
                } else {
                    // Click near a placed node to re-select it
                    int near = nodeNear(clickI);
                    if (near >= 0) m_selected = near;
                }
            }
        }
    }

    if (event.type == sf::Event::MouseButtonReleased &&
        event.mouseButton.button == sf::Mouse::Right)
        m_panning = false;

    if (event.type == sf::Event::MouseMoved && m_panning) {
        sf::Vector2i curr(event.mouseMove.x, event.mouseMove.y);
        sf::Vector2f viewSize = m_mapView.getSize();
        sf::Vector2f delta;
        delta.x = (m_panStart.x - curr.x) * (viewSize.x / (WIN_W - SIDEBAR_W));
        delta.y = (m_panStart.y - curr.y) * (viewSize.y / WIN_H);
        m_mapView.move(delta);
        m_panStart = curr;
    }

    (void)data;
}

void PinScreen::update(float /*dt*/) {}

void PinScreen::drawSidebar(sf::RenderWindow& window) {
    sf::RectangleShape bg(sf::Vector2f(SIDEBAR_W, WIN_H));
    bg.setFillColor(sf::Color(12, 10, 8, 245));
    bg.setOutlineColor(sf::Color(100, 80, 40));
    bg.setOutlineThickness(1.f);
    window.draw(bg);

    float scrollAreaH = WIN_H - BOT_AREA;
    int   visRows     = (int)(scrollAreaH / ITEM_H) + 1;

    sf::Text text;
    text.setFont(m_assets.font());

    for (int i = 0; i < visRows; i++) {
        int row = m_sideScroll + i;
        if (row >= TOTAL_ROWS) break;
        float y = i * ITEM_H;
        if (y + ITEM_H > scrollAreaH) break;

        int nodeId = rowToNode(row);
        if (nodeId < 0) {
            int cursor = 0, regionIdx = 0;
            for (int r = 0; r < 6; r++) {
                if (row == cursor) { regionIdx = r; break; }
                cursor += 1 + (REGIONS[r].last - REGIONS[r].first + 1);
            }
            sf::RectangleShape hdr(sf::Vector2f(SIDEBAR_W - 2.f, ITEM_H));
            hdr.setPosition(1.f, y);
            hdr.setFillColor(REGIONS[regionIdx].color);
            window.draw(hdr);
            text.setCharacterSize(10);
            text.setStyle(sf::Text::Bold);
            text.setFillColor(sf::Color(255, 255, 255, 230));
            text.setString(REGIONS[regionIdx].name);
            text.setPosition(5.f, y + 2.f);
            window.draw(text);
            text.setStyle(sf::Text::Regular);
        } else {
            if (nodeId == m_selected) {
                sf::RectangleShape sel(sf::Vector2f(SIDEBAR_W - 2.f, ITEM_H));
                sel.setPosition(1.f, y);
                sel.setFillColor(sf::Color(100, 80, 25, 200));
                window.draw(sel);
            }
            std::string name = m_graph.getNode(nodeId).name;
            if ((int)name.size() > 22) name = name.substr(0, 20) + "..";
            text.setCharacterSize(11);
            text.setString(name);
            text.setFillColor(m_positions.isPlaced(nodeId)
                              ? sf::Color(100, 210, 100)
                              : sf::Color(220, 140,  60));
            text.setPosition(10.f, y + 2.f);
            window.draw(text);
        }
    }

    sf::RectangleShape sep(sf::Vector2f(SIDEBAR_W, 1.f));
    sep.setPosition(0.f, scrollAreaH);
    sep.setFillColor(sf::Color(100, 80, 40, 180));
    window.draw(sep);

    float botY = scrollAreaH + 4.f;

    // Row 0: "Preset name:" + input
    m_presetLabel.setPosition(6.f, botY);
    window.draw(m_presetLabel);
    std::string display = m_presetName.empty() ? "WorldMap" : m_presetName;
    if (m_editingName) display += "_";
    m_presetInput.setString(display);
    m_presetInput.setPosition(90.f, botY + 1.f);
    m_presetInputRect = sf::FloatRect(88.f, botY - 2.f, SIDEBAR_W - 96.f, 18.f);
    window.draw(m_presetInput);

    // Row 1: [ Save ] — full width, only when all placed
    if (m_positions.allPlaced()) {
        sf::RectangleShape saveBg(sf::Vector2f(SIDEBAR_W - 12.f, 20.f));
        saveBg.setPosition(6.f, botY + 20.f);
        saveBg.setFillColor(sf::Color(30, 80, 30, 200));
        saveBg.setOutlineColor(sf::Color(80, 180, 80, 180));
        saveBg.setOutlineThickness(1.f);
        window.draw(saveBg);
        m_saveBtn.setPosition(12.f, botY + 22.f);
        m_saveBtnRect = sf::FloatRect(6.f, botY + 18.f, SIDEBAR_W - 12.f, 24.f);
        window.draw(m_saveBtn);
    }

    // Row 2: [ Back ]
    m_backBtn.setPosition(6.f, botY + 48.f);
    m_backBtnRect = sf::FloatRect(6.f, botY + 44.f, SIDEBAR_W - 12.f, 22.f);
    window.draw(m_backBtn);

    // Row 3: counter
    int placed = 0;
    for (int i = 0; i < NodePositions::NODE_COUNT; i++)
        if (m_positions.isPlaced(i)) placed++;
    sf::Text counter;
    counter.setFont(m_assets.font());
    counter.setCharacterSize(11);
    counter.setFillColor(sf::Color(180, 170, 140));
    counter.setString(std::to_string(placed) + " / 50 placed");
    counter.setPosition(6.f, botY + 70.f);
    window.draw(counter);
}

void PinScreen::drawMapArea(sf::RenderWindow& window) {
    window.draw(m_mapSprite);

    // Draw edges as thick lines
    int n = m_graph.size();
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (!m_graph.hasEdge(i, j)) continue;
            if (!m_positions.isPlaced(i) || !m_positions.isPlaced(j)) continue;
            sf::Vector2f a = m_positions.get(i), b = m_positions.get(j);
            sf::Vector2f dir = b - a;
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len < 0.5f) continue;
            dir /= len;
            sf::Vector2f perp(-dir.y * 1.5f, dir.x * 1.5f);
            sf::VertexArray quad(sf::Quads, 4);
            sf::Color ec(180, 140, 80, 130);
            quad[0].position = a + perp; quad[0].color = ec;
            quad[1].position = b + perp; quad[1].color = ec;
            quad[2].position = b - perp; quad[2].color = ec;
            quad[3].position = a - perp; quad[3].color = ec;
            window.draw(quad);
        }
    }

    auto regionColor = [](int id) -> sf::Color {
        if (id <=  7) return sf::Color( 70, 110, 180);
        if (id <= 15) return sf::Color(130,  80, 200);
        if (id <= 25) return sf::Color(210, 180,  60);
        if (id <= 33) return sf::Color( 60, 150,  60);
        if (id <= 41) return sf::Color(210, 140,  50);
        return             sf::Color(210,  60,  60);
    };

    sf::CircleShape circle(NODE_R);
    circle.setOrigin(NODE_R, NODE_R);
    circle.setOutlineThickness(2.f);
    circle.setOutlineColor(sf::Color(30, 20, 10));

    sf::Text label;
    label.setFont(m_assets.font());
    label.setCharacterSize(8);
    label.setFillColor(sf::Color(240, 230, 200, 180));

    for (int i = 0; i < n; i++) {
        if (!m_positions.isPlaced(i)) continue;
        sf::Vector2f pos = m_positions.get(i);
        circle.setPosition(pos);
        circle.setFillColor(i == m_selected ? sf::Color(255, 230, 60) : regionColor(i));
        window.draw(circle);
        label.setString(m_graph.getNode(i).name.substr(0, 12));
        label.setPosition(pos.x + NODE_R + 1.f, pos.y + 2.f);
        window.draw(label);
    }

    // Ghost dot following the cursor
    if (m_window) {
        sf::Vector2i mousePixel = sf::Mouse::getPosition(*m_window);
        sf::Vector2f mouseWorld = m_window->mapPixelToCoords(mousePixel, m_mapView);
        if (m_selected >= 0 && mousePixel.x >= (int)SIDEBAR_W) {
            sf::CircleShape ghost(NODE_R);
            ghost.setOrigin(NODE_R, NODE_R);
            ghost.setPosition(mouseWorld);
            ghost.setFillColor(sf::Color(255, 215, 60, 200));
            ghost.setOutlineColor(sf::Color(200, 160, 20));
            ghost.setOutlineThickness(2.f);
            window.draw(ghost);

            sf::Text ghostLabel;
            ghostLabel.setFont(m_assets.font());
            ghostLabel.setCharacterSize(9);
            ghostLabel.setFillColor(sf::Color(255, 245, 190, 220));
            ghostLabel.setString(m_graph.getNode(m_selected).name);
            ghostLabel.setPosition(mouseWorld.x + NODE_R + 2.f, mouseWorld.y - 12.f);
            window.draw(ghostLabel);
        }
    }
}

void PinScreen::draw(sf::RenderWindow& window) {
    m_window = &window;  // make window available to handleEvent via nodeNear()

    window.setView(m_mapView);
    drawMapArea(window);

    window.setView(window.getDefaultView());
    drawSidebar(window);

    sf::Text hint;
    hint.setFont(m_assets.font());
    hint.setCharacterSize(12);
    hint.setFillColor(sf::Color(200, 190, 160, 200));
    hint.setString("Click sidebar to select  |  Click map to place  |  Scroll: zoom  |  Right-drag: pan");
    hint.setPosition(SIDEBAR_W + 8.f, WIN_H - 20.f);
    window.draw(hint);
}

const NodePositions& PinScreen::positions() const { return m_positions; }
