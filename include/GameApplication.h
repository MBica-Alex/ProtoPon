#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <map>
#include "Game.h"
#include "AnimatedPosition.h"

enum class GameState {
    MENU,
    GAME
};

enum class UnitType {
    YARIPON,
    TATEPON,
    YUMIPON
};

class ArrowAnimation {
public:
    void start(float sX, float sY, float tX) {
        startX = sX;
        startY = sY;
        currentX = sX;
        currentY = sY;
        targetX = tX;
        timer = 0.0f;
        active = true;
    }

    void update(float dt) {
        if (!active) return;
        timer += dt;
        float t = timer / DURATION;
        if (t >= 1.0f) {
            t = 1.0f;
            active = false;
        }

        float ease = t * t * (3.0f - 2.0f * t);
        currentX = startX + (targetX - startX) * ease;
        
        float arcHeight = 100.0f;
        currentY = startY - (4.0f * arcHeight * t * (1.0f - t));

        float vx = (targetX - startX) * (6.0f * t * (1.0f - t));
        float vy = -4.0f * arcHeight * (1.0f - 2.0f * t);
        
        if (std::abs(vx) > 0.001f || std::abs(vy) > 0.001f) {
             rotation = std::atan2(vy, vx) * 180.0f / 3.14159f;
        }
    }

    [[nodiscard]] float getRotation() const { return rotation; }

    [[nodiscard]] bool isActive() const { return active; }
    [[nodiscard]] float getCurrentX() const { return currentX; }
    [[nodiscard]] float getCurrentY() const { return currentY; }

private:
    float startX = 0, startY = 0;
    float currentX = 0, currentY = 0;
    float targetX = 0;
    float timer = 0.0f;
    float rotation = 0.0f;
    bool active = false;
    const float DURATION = 0.6f;
};

class GameApplication {
public:
    GameApplication();
    void run();

private:
    void processEvents();
    void update(float dt);
    void render();
    void renderMenu();
    void renderStats();
    void renderWinScreen();
    void renderLoseScreen();

    const sf::Texture& getUnitTexture(UnitType type) const;
    

    float posToX(int pos) const;

    const float WINDOW_WIDTH = 1200.0f;
    const float WINDOW_HEIGHT = 800.0f;
    const float BATTLEFIELD_HEIGHT = WINDOW_HEIGHT * 0.75f;
    const float COMMAND_BAR_HEIGHT = WINDOW_HEIGHT * 0.25f;

    sf::RenderWindow m_window;
    sf::Font m_font;


    sf::Texture m_pataTexture;
    sf::Texture m_ponTexture;
    sf::Texture m_yariponTexture;
    sf::Texture m_tateponTexture;
    sf::Texture m_yumiponTexture;
    
    std::vector<sf::Texture*> m_pataponIcons;


    sf::Sprite m_pataSprite;
    sf::Sprite m_ponSprite;


    float m_pataAnimTimer = 0.0f;
    bool m_pataAnimActive = false;
    const float DRUM_ANIM_DURATION = 0.5f;

    float m_ponAnimTimer = 0.0f;
    bool m_ponAnimActive = false;

    ArrowAnimation m_arrowAnim;


    std::unique_ptr<Game> m_game;
    AnimatedPosition m_armyPos;
    std::map<const Enemy*, AnimatedPosition> m_enemyPositions;


    const float m_fieldLeft = 50.0f;

    const float m_fieldWidth = 1100.0f;
    const float m_unitRadius = 35.0f;
    const float m_fieldY = 600.0f - 35.0f + 10.0f;


    float m_victoryTimer = 0.0f;


    bool m_showStats = false;
    bool m_spaceKeyProcessed = false;


    float m_bossEventTimer = 0.0f;
    float m_bossEventAlpha = 0.0f;

    GameState m_state;
    std::vector<UnitType> m_selectedUnits;
    int m_menuSelectionIndex = 0;
};
