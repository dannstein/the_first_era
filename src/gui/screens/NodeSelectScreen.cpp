#include "gui/screens/NodeSelectScreen.h"
#include "core/Node.h"
#include <SFML/Window/Keyboard.hpp>
#include <algorithm>

static constexpr float WIN_W   = 1600.f;
static constexpr float WIN_H   = 900.f;
static constexpr float IMG_W   = 1672.f;
static constexpr float IMG_H   = 941.f;

struct RegionInfo {
    const char* name;
    int first, last;
    sf::Color color;
};

static const RegionInfo REGIONS[6] = {
    {"REGION I  — Eryndorheim",   0,  7, sf::Color( 70, 110, 180, 255)},
    {"REGION II — Nocthyr Vale",  8, 15, sf::Color(120,  60, 170, 255)},
    {"REGION III — Aurelion",    16, 25, sf::Color(200, 165,  40, 255)},
    {"REGION IV — Morvhal Mire", 26, 33, sf::Color( 60, 110,  60, 255)},
    {"REGION V  — Solkarath",    34, 41, sf::Color(210, 130,  40, 255)},
    {"REGION VI — Drakmord",     42, 49, sf::Color(190,  50,  50, 255)},
};

NodeSelectScreen::NodeSelectScreen(AssetManager& assets, const Graph& graph)
    : m_assets(assets)
{
    m_mapSprite.setTexture(m_assets.mapTexture());
    m_mapSprite.setScale(WIN_W / IMG_W, WIN_H / IMG_H);

    m_title.setFont(m_assets.font());
    m_title.setString("Select Starting Node");
    m_title.setCharacterSize(26);
    m_title.setFillColor(sf::Color(220, 200, 150));
    m_title.setStyle(sf::Text::Bold);

    m_subtitle.setFont(m_assets.font());
    m_subtitle.setString("The route will always begin (and end) here.");
    m_subtitle.setCharacterSize(14);
    m_subtitle.setFillColor(sf::Color(170, 160, 130));

    buildItemList(graph);
    scrollToSelected();
}

void NodeSelectScreen::buildItemList(const Graph& graph) {
    m_items.clear();
    for (const auto& r : REGIONS) {
        ListItem hdr;
        hdr.isHeader = true;
        hdr.nodeId   = -1;
        hdr.label    = r.name;
        hdr.color    = r.color;
        m_items.push_back(hdr);

        for (int n = r.first; n <= r.last; n++) {
            ListItem item;
            item.isHeader = false;
            item.nodeId   = n;
            item.label    = std::to_string(n) + "  " + graph.getNode(n).name;
            item.color    = r.color;
            m_items.push_back(item);
        }
    }
}

void NodeSelectScreen::clampScroll() {
    int maxScroll = std::max(0, (int)m_items.size() - (int)(NodeSelectScreen::LIST_H / ROW_H));
    m_scroll = std::max(0, std::min(m_scroll, maxScroll));
}

void NodeSelectScreen::scrollToSelected() {
    // Find the item index for m_selectedNode
    int visRows = (int)(LIST_H / ROW_H);
    for (int i = 0; i < (int)m_items.size(); i++) {
        if (!m_items[i].isHeader && m_items[i].nodeId == m_selectedNode) {
            // Scroll so that this item is visible
            if (i < m_scroll)
                m_scroll = i;
            else if (i >= m_scroll + visRows)
                m_scroll = i - visRows + 1;
            break;
        }
    }
    clampScroll();
}

void NodeSelectScreen::handleEvent(const sf::Event& event, AppState& next, TransitionData& data) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            next = AppState::AlgoSelect;
            return;
        }
        if (event.key.code == sf::Keyboard::Up) {
            // Find previous node item
            for (int i = (int)m_items.size() - 1; i >= 0; i--) {
                if (!m_items[i].isHeader && m_items[i].nodeId < m_selectedNode) {
                    m_selectedNode = m_items[i].nodeId;
                    scrollToSelected();
                    break;
                }
            }
        }
        if (event.key.code == sf::Keyboard::Down) {
            for (int i = 0; i < (int)m_items.size(); i++) {
                if (!m_items[i].isHeader && m_items[i].nodeId > m_selectedNode) {
                    m_selectedNode = m_items[i].nodeId;
                    scrollToSelected();
                    break;
                }
            }
        }
        if (event.key.code == sf::Keyboard::Return) {
            data.fixedStart = m_selectedNode;
            next = data.benchmarkMode ? AppState::Benchmark : AppState::Game;
        }
    }

    if (event.type == sf::Event::MouseWheelScrolled) {
        m_scroll -= (int)event.mouseWheelScroll.delta;
        clampScroll();
    }

    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f click((float)event.mouseButton.x, (float)event.mouseButton.y);

        // Click in the list
        if (m_listRect.contains(click)) {
            int row = (int)((click.y - m_listRect.top) / ROW_H) + m_scroll;
            if (row >= 0 && row < (int)m_items.size() && !m_items[row].isHeader) {
                m_selectedNode = m_items[row].nodeId;
            }
            return;
        }

        if (m_defaultBtnRect.contains(click)) {
            data.fixedStart = 0;
            next = data.benchmarkMode ? AppState::Benchmark : AppState::Game;
        } else if (m_startBtnRect.contains(click)) {
            data.fixedStart = m_selectedNode;
            next = data.benchmarkMode ? AppState::Benchmark : AppState::Game;
        } else if (m_backBtnRect.contains(click)) {
            next = AppState::AlgoSelect;
        }
    }
}

void NodeSelectScreen::update(float /*dt*/) {}

void NodeSelectScreen::draw(sf::RenderWindow& window) {
    window.draw(m_mapSprite);

    sf::RectangleShape overlay(sf::Vector2f(WIN_W, WIN_H));
    overlay.setFillColor(sf::Color(0, 0, 0, 130));
    window.draw(overlay);

    float px = (WIN_W - PANEL_W) / 2.f;
    float py = (WIN_H - PANEL_H) / 2.f;

    // Panel background
    sf::RectangleShape panel(sf::Vector2f(PANEL_W, PANEL_H));
    panel.setPosition(px, py);
    panel.setFillColor(sf::Color(10, 8, 6, 230));
    panel.setOutlineColor(sf::Color(150, 120, 60));
    panel.setOutlineThickness(2.f);
    window.draw(panel);

    // Title
    auto tb = m_title.getLocalBounds();
    m_title.setOrigin(tb.left + tb.width / 2.f, tb.top);
    m_title.setPosition(px + PANEL_W / 2.f, py + 14.f);
    window.draw(m_title);

    // Subtitle
    auto sb = m_subtitle.getLocalBounds();
    m_subtitle.setOrigin(sb.left + sb.width / 2.f, sb.top);
    m_subtitle.setPosition(px + PANEL_W / 2.f, py + 48.f);
    window.draw(m_subtitle);

    // Divider
    sf::RectangleShape divider(sf::Vector2f(PANEL_W - 20.f, 1.f));
    divider.setPosition(px + 10.f, py + 70.f);
    divider.setFillColor(sf::Color(100, 80, 40, 180));
    window.draw(divider);

    // ── Scrollable list ──────────────────────────────────────────────────────
    float listX = px + 10.f;
    float listY = py + 76.f;
    float listW = PANEL_W - 20.f;
    m_listRect  = sf::FloatRect(listX, listY, listW, LIST_H);

    // List background
    sf::RectangleShape listBg(sf::Vector2f(listW, LIST_H));
    listBg.setPosition(listX, listY);
    listBg.setFillColor(sf::Color(18, 14, 10, 200));
    window.draw(listBg);

    // Draw list items using a viewport for clipping
    auto savedView = window.getView();
    sf::View listView(sf::FloatRect(0.f, 0.f, listW, LIST_H));
    listView.setViewport(sf::FloatRect(listX / WIN_W, listY / WIN_H,
                                       listW / WIN_W, LIST_H / WIN_H));
    window.setView(listView);

    int visRows = (int)(LIST_H / ROW_H) + 2; // +2 for partial rows
    sf::Text rowText;
    rowText.setFont(m_assets.font());
    rowText.setCharacterSize(13);

    for (int i = m_scroll; i < (int)m_items.size() && i < m_scroll + visRows; i++) {
        float rowY = (i - m_scroll) * ROW_H;
        const auto& item = m_items[i];

        if (item.isHeader) {
            // Region header: tinted background + bold label
            sf::Color bg = item.color;
            bg.a = 60;
            sf::RectangleShape hdrBg(sf::Vector2f(listW, ROW_H - 1.f));
            hdrBg.setPosition(0.f, rowY);
            hdrBg.setFillColor(bg);
            window.draw(hdrBg);

            rowText.setString(item.label);
            rowText.setFillColor(item.color);
            rowText.setStyle(sf::Text::Bold);
            rowText.setCharacterSize(12);
            rowText.setPosition(6.f, rowY + 3.f);
            window.draw(rowText);
            rowText.setStyle(sf::Text::Regular);
            rowText.setCharacterSize(13);
        } else {
            bool selected = (item.nodeId == m_selectedNode);

            if (selected) {
                sf::RectangleShape sel(sf::Vector2f(listW, ROW_H - 1.f));
                sel.setPosition(0.f, rowY);
                sf::Color selCol = item.color;
                selCol.a = 100;
                sel.setFillColor(selCol);
                window.draw(sel);
            }

            sf::Color textCol = selected ? sf::Color(255, 240, 100) : sf::Color(200, 190, 165);
            rowText.setString(item.label);
            rowText.setFillColor(textCol);
            rowText.setPosition(18.f, rowY + 3.f);
            window.draw(rowText);

            if (selected) {
                // Small arrow indicator
                sf::Text arrow;
                arrow.setFont(m_assets.font());
                arrow.setCharacterSize(13);
                arrow.setFillColor(sf::Color(255, 240, 100));
                arrow.setString(">");
                arrow.setPosition(6.f, rowY + 3.f);
                window.draw(arrow);
            }
        }
    }

    // Scroll bar
    if ((int)m_items.size() > (int)(LIST_H / ROW_H)) {
        float totalH = m_items.size() * ROW_H;
        float barH   = std::max(20.f, LIST_H * LIST_H / totalH);
        float barY   = m_scroll * ROW_H * LIST_H / totalH;
        sf::RectangleShape bar(sf::Vector2f(4.f, barH));
        bar.setPosition(listW - 5.f, barY);
        bar.setFillColor(sf::Color(140, 120, 70, 180));
        window.draw(bar);
    }

    window.setView(savedView);
    // ─────────────────────────────────────────────────────────────────────────

    // Divider above buttons
    float botY = listY + LIST_H + 6.f;
    sf::RectangleShape div2(sf::Vector2f(PANEL_W - 20.f, 1.f));
    div2.setPosition(px + 10.f, botY);
    div2.setFillColor(sf::Color(100, 80, 40, 180));
    window.draw(div2);

    botY += 8.f;

    // Helper for drawing a button
    auto drawBtn = [&](const std::string& label, float bx, float by, float bw, float bh,
                        sf::Color fill, sf::Color txtCol, sf::FloatRect& outRect) {
        sf::RectangleShape bg(sf::Vector2f(bw, bh));
        bg.setPosition(bx, by);
        bg.setFillColor(fill);
        bg.setOutlineColor(sf::Color(150, 120, 60, 180));
        bg.setOutlineThickness(1.f);
        window.draw(bg);

        sf::Text btn;
        btn.setFont(m_assets.font());
        btn.setCharacterSize(14);
        btn.setFillColor(txtCol);
        btn.setString(label);
        auto bb = btn.getLocalBounds();
        btn.setOrigin(bb.left + bb.width / 2.f, bb.top + bb.height / 2.f);
        btn.setPosition(bx + bw / 2.f, by + bh / 2.f);
        window.draw(btn);

        outRect = sf::FloatRect(bx, by, bw, bh);
    };

    float btnH = 28.f;
    float col1W = (PANEL_W - 30.f) * 0.55f;
    float col2W = (PANEL_W - 30.f) * 0.27f;
    float col3W = (PANEL_W - 30.f) * 0.14f;

    drawBtn("Use Default (Frozen Fortress)",
            px + 10.f, botY, col1W, btnH,
            sf::Color(20, 50, 80, 200), sf::Color(160, 200, 240),
            m_defaultBtnRect);

    drawBtn("Start with Selected",
            px + 10.f + col1W + 5.f, botY, col2W, btnH,
            sf::Color(30, 70, 30, 200), sf::Color(140, 220, 120),
            m_startBtnRect);

    drawBtn("Back",
            px + 10.f + col1W + col2W + 10.f, botY, col3W, btnH,
            sf::Color(40, 30, 20, 200), sf::Color(180, 160, 120),
            m_backBtnRect);

    // Show currently selected node name below buttons
    float infoY = botY + btnH + 8.f;
    std::string selName = "Node " + std::to_string(m_selectedNode);
    // Find label for selectedNode
    for (const auto& it : m_items) {
        if (!it.isHeader && it.nodeId == m_selectedNode) {
            selName = it.label;
            break;
        }
    }
    sf::Text selInfo;
    selInfo.setFont(m_assets.font());
    selInfo.setCharacterSize(13);
    selInfo.setFillColor(sf::Color(200, 185, 140));
    selInfo.setString("Selected: " + selName);
    auto sib = selInfo.getLocalBounds();
    selInfo.setOrigin(sib.left + sib.width / 2.f, sib.top);
    selInfo.setPosition(px + PANEL_W / 2.f, infoY);
    window.draw(selInfo);
}
