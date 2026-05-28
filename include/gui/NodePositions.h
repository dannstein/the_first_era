#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class NodePositions {
public:
    static constexpr int NODE_COUNT = 50;  // default / WorldMap size

    explicit NodePositions(int size = NODE_COUNT);

    bool loadFromFile(const std::string& path);
    void saveToFile(const std::string& path) const;

    int          size()  const;
    bool         isPlaced(int id) const;
    bool         allPlaced() const;
    sf::Vector2f get(int id) const;
    void         set(int id, sf::Vector2f pos);

    void applyFallback();  // only valid when size() == NODE_COUNT

    static std::vector<std::string> listPresets(const std::string& configDir);
    static std::string presetPath(const std::string& configDir, const std::string& name);

private:
    int                       m_size;
    std::vector<sf::Vector2f> m_pos;
    std::vector<bool>         m_placed;
};
