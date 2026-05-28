#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "gui/Screen.h"
#include "gui/AssetManager.h"

class ParamConfigScreen : public Screen {
public:
    ParamConfigScreen(AssetManager& assets, int algoChoice);

    void handleEvent(const sf::Event& event, AppState& next, TransitionData& data) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    static constexpr float WIN_W = 1600.f;
    static constexpr float WIN_H = 900.f;
    static constexpr float IMG_W = 1672.f;
    static constexpr float IMG_H = 941.f;

    struct Field {
        std::string label;
        std::string hint;        // shown as "(default: X / range hint)"
        std::string value;       // current editable string
        std::string defaultVal;  // for Reset
        bool        isFloat;     // allows '.' in input
    };

    AssetManager&       m_assets;
    sf::Sprite          m_mapSprite;
    int                 m_algoChoice;
    std::vector<Field>  m_fields;
    int                 m_focused = -1;  // index of focused field (-1 = none)

    // Hit rects set in draw()
    std::vector<sf::FloatRect> m_fieldRects;
    sf::FloatRect              m_resetBtnRect;
    sf::FloatRect              m_continueBtnRect;
    sf::FloatRect              m_backBtnRect;

    void buildFields();
    void resetToDefaults();
    void applyToData(TransitionData& data) const;

    // Benchmark section
    bool        m_benchmarkEnabled = false;
    std::string m_runsStr          = "20";
    bool        m_runsFocused      = false;
    sf::FloatRect m_benchmarkToggleRect;
    sf::FloatRect m_runsBoxRect;

    static const char* algoName(int choice);
};
