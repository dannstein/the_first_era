#include "gui/NodePositions.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <iostream>

NodePositions::NodePositions() {
    m_placed.fill(false);
    m_pos.fill({0.f, 0.f});
}

bool NodePositions::loadFromFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return false;

    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);
        int id; float x, y;
        if (ss >> id >> x >> y && id >= 0 && id < NODE_COUNT) {
            m_pos[id]    = {x, y};
            m_placed[id] = true;
        }
    }
    return true;
}

void NodePositions::saveToFile(const std::string& path) const {
    std::ofstream f(path);
    f << "# The First Era — node positions (image-space pixels)\n";
    f << "# id x y\n";
    for (int i = 0; i < NODE_COUNT; i++)
        if (m_placed[i])
            f << i << " " << m_pos[i].x << " " << m_pos[i].y << "\n";
}

bool         NodePositions::isPlaced(int id) const { return m_placed[id]; }
bool         NodePositions::allPlaced() const {
    for (bool b : m_placed) if (!b) return false;
    return true;
}
sf::Vector2f NodePositions::get(int id)          const { return m_pos[id]; }
void         NodePositions::set(int id, sf::Vector2f p) { m_pos[id] = p; m_placed[id] = true; }

void NodePositions::applyFallback() {
    // Approximate positions on the 1672x941 image by region
    // Region I: Eryndorheim (0-7) — top-left
    static const sf::Vector2f fallback[NODE_COUNT] = {
        {150,  90}, {230,  70}, {310,  90}, {370, 140}, {280, 180},  // 0-4
        {190, 200}, {120, 160}, {340, 200},                          // 5-7

        // Region II: Nocthyr Vale (8-15) — mid-left
        {130, 340}, {220, 310}, {320, 340}, {420, 310}, {380, 420},  // 8-12
        {260, 450}, {160, 420}, {300, 490},                          // 13-15

        // Region III: Aurelion (16-25) — center
        {620, 200}, {720, 170}, {820, 200}, {920, 230}, {840, 310},  // 16-20
        {730, 340}, {620, 310}, {950, 310}, {680, 410}, {800, 410},  // 21-25

        // Region IV: Morvhal Mire (26-33) — bottom-center-left
        {420, 590}, {530, 560}, {640, 590}, {580, 680}, {470, 700},  // 26-30
        {360, 680}, {480, 790}, {620, 770},                          // 31-33

        // Region V: Solkarath (34-41) — right
        {1150, 210}, {1270, 180}, {1390, 210}, {1460, 300}, {1380, 390},  // 34-38
        {1260, 420}, {1150, 390}, {1310, 300},                            // 39-41 (wait, 35-41 is 7 nodes, region V is 34-41 = 8 nodes)

        // Region VI: Drakmord (42-49) — bottom-right
        {1150, 600}, {1270, 570}, {1390, 600}, {1460, 690}, {1380, 770},  // 42-46
        {1260, 800}, {1150, 770}, {1320, 690},                            // 47-49
    };

    for (int i = 0; i < NODE_COUNT; i++) {
        if (!m_placed[i]) {
            m_pos[i]    = fallback[i];
            m_placed[i] = true;
        }
    }
}

std::vector<std::string> NodePositions::listPresets(const std::string& configDir) {
    std::vector<std::string> result;
    namespace fs = std::filesystem;

    if (!fs::exists(configDir)) return result;

    for (auto& entry : fs::directory_iterator(configDir)) {
        std::string name = entry.path().filename().string();
        if (name == "positions.txt") {
            result.push_back("WorldMap");
        } else if (name.size() > 11 && name.substr(0, 7) == "preset_"
                   && name.substr(name.size() - 4) == ".txt") {
            result.push_back(name.substr(7, name.size() - 11));
        }
    }

    std::sort(result.begin(), result.end());
    return result;
}

std::string NodePositions::presetPath(const std::string& configDir, const std::string& name) {
    if (name == "WorldMap" || name.empty())
        return configDir + "/positions.txt";
    return configDir + "/preset_" + name + ".txt";
}
