#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "gui/Screen.h"
#include "gui/AssetManager.h"

class OverallBenchmarkSetupScreen : public Screen {
public:
    explicit OverallBenchmarkSetupScreen(AssetManager& assets);

    void handleEvent(const sf::Event& event, AppState& next, TransitionData& data) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    AssetManager& m_assets;
    sf::Sprite    m_mapSprite;

    // General params
    std::string m_runsStr       = "20";
    bool        m_useRandom     = false;
    std::string m_nodeCountStr  = "20";
    bool        m_fixedNone     = false;   // true = no fixed start
    std::string m_startNodeStr  = "0";     // used when !m_fixedNone

    // Algorithm params
    std::string m_hctTMaxStr    = "5";
    std::string m_saTIStr       = "1000.0";
    std::string m_saTFStr       = "0.001";
    std::string m_saFRStr       = "0.995";
    std::string m_gaPopStr      = "200";
    std::string m_gaGenStr      = "500";
    std::string m_gaMutStr      = "8";
    std::string m_gaTourStr     = "3";
    std::string m_gaEliteStr    = "10";
    std::string m_gaBreedStr    = "80";
    std::string m_gaStagStr     = "75";

    // Focus tracking: index into m_inputFields
    int m_focused = -1;

    struct InputField {
        std::string* value;
        bool        isFloat;
        sf::FloatRect rect;
    };
    std::vector<InputField> m_inputFields;  // built each draw call

    // Button rects (set during draw)
    sf::FloatRect m_defaultGraphRect;
    sf::FloatRect m_randomGraphRect;
    sf::FloatRect m_fixedStartToggleRect;
    sf::FloatRect m_startBtnRect;
    sf::FloatRect m_backBtnRect;

    void applyToData(TransitionData& data) const;

    static double safeDouble(const std::string& s, double fallback);
    static int    safeInt(const std::string& s, int fallback, int lo = 1, int hi = 100000);
};
