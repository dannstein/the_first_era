#include "gui/AssetManager.h"
#include <iostream>

bool AssetManager::load(const std::string& projectRoot) {
    if (!m_font.loadFromFile(projectRoot + "/assets/fonts/DejaVuSans.ttf")) {
        std::cerr << "Failed to load font\n";
        return false;
    }
    if (!m_map.loadFromFile(projectRoot + "/sprites/map.png")) {
        std::cerr << "Failed to load map texture\n";
        return false;
    }
    m_map.setSmooth(true);
    return true;
}

sf::Font& AssetManager::font() { return m_font; }
sf::Texture& AssetManager::mapTexture() { return m_map; }
