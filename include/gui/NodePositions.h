#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <string>
#include <vector>

class NodePositions {
public:
    static constexpr int NODE_COUNT = 50;

    NodePositions();

    bool loadFromFile(const std::string& path);
    void saveToFile(const std::string& path) const;

    bool         isPlaced(int id) const;
    bool         allPlaced() const;
    sf::Vector2f get(int id) const;
    void         set(int id, sf::Vector2f pos);

    void applyFallback();

    static std::vector<std::string> listPresets(const std::string& configDir);
    static std::string presetPath(const std::string& configDir, const std::string& name);

private:
    std::array<sf::Vector2f, NODE_COUNT> m_pos;
    std::array<bool,          NODE_COUNT> m_placed;
};
