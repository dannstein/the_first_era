#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class AssetManager {
public:
    bool load(const std::string& projectRoot);

    sf::Font&    font();
    sf::Texture& mapTexture();

private:
    sf::Font    m_font;
    sf::Texture m_map;
};
