#include <SFML/Graphics.hpp>
#include <sstream>
#include <iostream>
#include <cmath>
#include <map>
#include <set>
#include <optional>
#include <cstdint>
#include <algorithm>
#include "Game.h"
#include "Boss.h"
#include "GameException.h"
#include "GameConfig.h"

float easeOutCubic(float t) {
    return 1.0f - std::pow(1.0f - t, 3.0f);
}

float lerp(float start, float end, float t) {
    return start + (end - start) * t;
}

class AnimatedPosition {
public:
    float currentX = 0;
    float currentY = 0;
    float targetX = 0;
    float targetY = 0;
    float progress = 1.0f;
    static constexpr float ANIMATION_SPEED = 3.0f;

    bool isSpawning = false;
    float spawnTimer = 0.0f;
    float currentScale = 1.0f;
    static constexpr float SPAWN_DURATION = 0.6f;

    void setTarget(float x, float y) {
        if (std::abs(targetX - x) > 0.1f || std::abs(targetY - y) > 0.1f) {
            targetX = x;
            targetY = y;
            progress = 0.0f;
        }
    }

    void startSpawn() {
        isSpawning = true;
        spawnTimer = 0.0f;
        currentScale = 0.0f;
    }

    void update(float dt) {
        if (progress < 1.0f) {
            progress += dt * ANIMATION_SPEED;
            if (progress > 1.0f) progress = 1.0f;
            float eased = easeOutCubic(progress);
            currentX = lerp(currentX + (targetX - currentX) * (1.0f - eased), targetX, eased);
            currentY = lerp(currentY + (targetY - currentY) * (1.0f - eased), targetY, eased);
        } else {
            currentX = targetX;
            currentY = targetY;
        }

        if (isSpawning) {
            spawnTimer += dt;
            float t = spawnTimer / SPAWN_DURATION;
            if (t >= 1.0f) {
                t = 1.0f;
                isSpawning = false;
                currentScale = 1.0f;
            } else {
                const float c1 = 3.0f; 
                const float c3 = c1 + 1.0f;
                currentScale = 1.0f + c3 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f);
            }
        }
    }

    void snapTo(float x, float y) {
        currentX = targetX = x;
        currentY = targetY = y;
        progress = 1.0f;
    }
};

class ArrowAnimation {
public:
    float startX, startY;
    float currentX, currentY;
    float targetX;
    float timer = 0.0f;
    bool active = false;
    const float DURATION = 0.6f; 

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
            active = false;
            return;
        }

        float ease = t * t * t; 
        currentX = startX + (targetX - startX) * ease;
    }
};

int main() {
    try {
        const float WINDOW_WIDTH = 1200.0f;
        const float WINDOW_HEIGHT = 800.0f;
        const float BATTLEFIELD_HEIGHT = WINDOW_HEIGHT * 0.75f;
        const float COMMAND_BAR_HEIGHT = WINDOW_HEIGHT * 0.25f;

        sf::RenderWindow window(sf::VideoMode({static_cast<unsigned>(WINDOW_WIDTH), static_cast<unsigned>(WINDOW_HEIGHT)}), "PROTOPON");
        window.setFramerateLimit(60);

        sf::Image icon;
        if (icon.loadFromFile("assets/icon.png")) {
            window.setIcon(icon.getSize(), icon.getPixelsPtr());
        }

        sf::Font font;
        if (!font.openFromFile("assets/pata_font.ttf")) {
            if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
                throw ResourceLoadException("Failed to load font: assets/pata_font.ttf or C:/Windows/Fonts/arial.ttf");
            }
        }

        sf::Texture pataTexture;
        if (!pataTexture.loadFromFile("assets/pata.png")) {
            throw ResourceLoadException("Failed to load texture: assets/pata.png");
        }
        sf::Sprite pataSprite(pataTexture);
        pataSprite.setOrigin(sf::Vector2f(pataTexture.getSize()) / 2.0f);
        pataSprite.setPosition({80, BATTLEFIELD_HEIGHT / 2});
        float pataAnimTimer = 0.0f;
        bool pataAnimActive = false;
        const float DRUM_ANIM_DURATION = 0.5f;

        sf::Texture ponTexture;
        if (!ponTexture.loadFromFile("assets/pon.png")) {
            throw ResourceLoadException("Failed to load texture: assets/pon.png");
        }
        sf::Sprite ponSprite(ponTexture);
        ponSprite.setOrigin(sf::Vector2f(ponTexture.getSize()) / 2.0f);
        ponSprite.setPosition({WINDOW_WIDTH - 80, BATTLEFIELD_HEIGHT / 2});
        float ponAnimTimer = 0.0f;
        bool ponAnimActive = false;

        sf::Texture yariponTexture, tateponTexture, yumiponTexture;
        if (!yariponTexture.loadFromFile("assets/yaripon.png")) {
            throw ResourceLoadException("Failed to load texture: assets/yaripon.png");
        }
        if (!tateponTexture.loadFromFile("assets/tatepon.png")) {
            throw ResourceLoadException("Failed to load texture: assets/tatepon.png");
        }
        if (!yumiponTexture.loadFromFile("assets/yumipon.png")) {
            throw ResourceLoadException("Failed to load texture: assets/yumipon.png");
        }
        if (!yumiponTexture.loadFromFile("assets/yumipon.png")) {
            throw ResourceLoadException("Failed to load texture: assets/yumipon.png");
        }
        std::vector<sf::Texture*> pataponIcons = {&tateponTexture, &yariponTexture, &yumiponTexture};

        sf::Texture arrowTexture;
        if(!arrowTexture.loadFromFile("assets/arrow.png")) {
             throw ResourceLoadException("Failed to load texture: assets/arrow.png");
        }
        sf::Sprite arrowSprite(arrowTexture);
        arrowSprite.setOrigin(sf::Vector2f(arrowTexture.getSize()) / 2.0f);
        
        ArrowAnimation arrowAnim;

        std::vector<Patapon> soldiers = GameConfig::loadSoldiers("assets/game_config.txt");
        Army army(soldiers, 0);

        std::vector<std::unique_ptr<Enemy>> initialEnemies;
        Game game(army, std::move(initialEnemies));

        const float fieldLeft = 50.0f;
        const float fieldRight = WINDOW_WIDTH - 50.0f;
        const float fieldWidth = fieldRight - fieldLeft;
        const float unitRadius = 35.0f;
        const float fieldY = BATTLEFIELD_HEIGHT - unitRadius + 10;

        auto posToX = [&](int pos) {
            return fieldLeft + (static_cast<float>(pos) / (GameConstants::MAP_SIZE - 1)) * fieldWidth;
        };

        AnimatedPosition armyPos;
        armyPos.snapTo(posToX(game.getArmy().getPosition()), fieldY);

        std::map<const Enemy*, AnimatedPosition> enemyPositions;
        for (const auto& e : game.getEnemies()) {
            AnimatedPosition pos;
            pos.snapTo(posToX(e->getPos()), fieldY);
            enemyPositions[e.get()] = pos;
        }

        sf::Clock clock;


        
        float victoryTimer = 0.0f;

        while (window.isOpen()) {
            float dt = clock.restart().asSeconds();

            while (const auto event = window.pollEvent()) {
                if (event->is<sf::Event::Closed>()) {
                    window.close();
                }
                
                if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                    if (!game.isBossEventActive() && !game.isVictoryMarching()) {
                        if (keyPressed->code == sf::Keyboard::Key::A) {
                            game.processInput("pa");
                            pataAnimActive = true;
                            pataAnimTimer = 0.0f;
                        } else if (keyPressed->code == sf::Keyboard::Key::D) {
                            game.processInput("po");
                            ponAnimActive = true;
                            ponAnimTimer = 0.0f;
                        }
                    } 
                    if (keyPressed->code == sf::Keyboard::Key::Escape) {
                        window.close();
                    }
                }
            }

            if (game.isVictoryMarching()) {
                 victoryTimer += dt;
                 
                 if (victoryTimer < 1.0f) {
                     armyPos.setTarget(posToX(game.getGoal()), fieldY);
                     armyPos.update(dt);
                 } 
                 else if (victoryTimer < 3.0f) {
                     float marchProgress = (victoryTimer - 1.0f) / 2.0f;
                     float startX = posToX(game.getGoal());
                     float endX = startX + 150.0f;
                     
                     float currentMarchX = startX + (endX - startX) * marchProgress;
                     
                     armyPos.setTarget(currentMarchX, fieldY);
                     armyPos.update(dt * 0.5f);
                 }
                 else {
                     game.finishVictoryMarch();
                 }
            } else {
                armyPos.setTarget(posToX(game.getArmy().getPosition()), fieldY);
                armyPos.update(dt);
            }

            for (const auto& e : game.getEnemies()) {
                if (enemyPositions.find(e.get()) == enemyPositions.end()) {
                    AnimatedPosition pos;
                    pos.snapTo(posToX(e->getPos()), fieldY);
                    pos.startSpawn();
                    enemyPositions[e.get()] = pos;
                }
                enemyPositions[e.get()].setTarget(posToX(e->getPos()), fieldY);
                enemyPositions[e.get()].update(dt);
            }

            if (pataAnimActive) {
                pataAnimTimer += dt;
                if (pataAnimTimer >= DRUM_ANIM_DURATION) {
                    pataAnimActive = false;
                }
            }

            if (game.pollAttackTriggered()) {
                 float sX = armyPos.currentX;
                 float sY = armyPos.currentY; 
                 float tileWidth = fieldWidth / (GameConstants::MAP_SIZE - 1);
                 float tX = sX + 3.0f * tileWidth;
                 arrowAnim.start(sX, sY, tX);
            }
            arrowAnim.update(dt);

            if (ponAnimActive) {
                ponAnimTimer += dt;
                if (ponAnimTimer >= DRUM_ANIM_DURATION) {
                    ponAnimActive = false;
                }
            }
            
            game.pollAttackTriggered(); 

            window.clear(sf::Color(20, 20, 40));

            sf::RectangleShape sky(sf::Vector2f(WINDOW_WIDTH, BATTLEFIELD_HEIGHT));
            sky.setPosition({0, 0});
            sky.setFillColor(sf::Color(100, 150, 220));
            window.draw(sky);

            const float hpCircleRadius = 30.0f;
            const float hpBarWidth = 50.0f;
            const float hpBarHeight = 8.0f;
            float hpStartY = 15.0f;
            float hpSpacing = 70.0f;
            
            const auto& currentSoldiers = game.getArmy().getSoldiers();
            std::vector<size_t> displayOrder = {2, 1, 0};
            
            for (size_t displayIdx = 0; displayIdx < 3 && displayIdx < currentSoldiers.size(); ++displayIdx) {
                size_t i = displayOrder[displayIdx];
                if (i >= currentSoldiers.size()) continue;
                
                float xPos = 50.0f + displayIdx * hpSpacing;
                float yPos = hpStartY + hpCircleRadius;

                sf::CircleShape circle(hpCircleRadius);
                circle.setOrigin({hpCircleRadius, hpCircleRadius});
                circle.setPosition({xPos, yPos});
                circle.setFillColor(sf::Color(255, 255, 255, 100));
                circle.setOutlineColor(sf::Color::White);
                circle.setOutlineThickness(2);
                window.draw(circle);

                sf::Sprite iconSprite(*pataponIcons[i]);
                float iconScale = (hpCircleRadius * 1.6f) / static_cast<float>(pataponIcons[i]->getSize().x);
                iconSprite.setScale({iconScale, iconScale});
                iconSprite.setOrigin(sf::Vector2f(pataponIcons[i]->getSize()) / 2.0f);
                iconSprite.setPosition({xPos, yPos});
                window.draw(iconSprite);

                float barX = xPos - hpBarWidth / 2;
                float barY = yPos + hpCircleRadius + 5;

                sf::RectangleShape hpBack(sf::Vector2f(hpBarWidth, hpBarHeight));
                hpBack.setPosition({barX, barY});
                hpBack.setFillColor(sf::Color::Black);
                window.draw(hpBack);

                float hpPercent = static_cast<float>(currentSoldiers[i]->getHP()) / static_cast<float>(currentSoldiers[i]->getMaxHP());
                hpPercent = std::clamp(hpPercent, 0.0f, 1.0f);

                std::uint8_t r = static_cast<std::uint8_t>((1.0f - hpPercent) * 255);
                std::uint8_t g = static_cast<std::uint8_t>(hpPercent * 255);
                sf::Color hpColor(r, g, 0);

                sf::RectangleShape hpBar(sf::Vector2f(hpBarWidth * hpPercent, hpBarHeight));
                hpBar.setPosition({barX, barY});
                hpBar.setFillColor(hpColor);
                window.draw(hpBar);
            }

            float goalX = posToX(game.getGoal());
            
            sf::Color totemColor = sf::Color::Black;
            sf::Color accentColor = (game.hasWon() || game.isVictoryMarching()) ? sf::Color::Green : sf::Color::Red;
            
            float groundY = BATTLEFIELD_HEIGHT;

            sf::ConvexShape base;
            base.setPointCount(4);
            base.setPoint(0, sf::Vector2f(goalX - 30, groundY));
            base.setPoint(1, sf::Vector2f(goalX + 30, groundY));
            base.setPoint(2, sf::Vector2f(goalX + 20, groundY - 30));
            base.setPoint(3, sf::Vector2f(goalX - 20, groundY - 30));
            base.setFillColor(totemColor);
            window.draw(base);

            sf::RectangleShape lowerBody(sf::Vector2f(40, 50));
            lowerBody.setOrigin({20, 50});
            lowerBody.setPosition({goalX, groundY - 30});
            lowerBody.setFillColor(totemColor);
            window.draw(lowerBody);

            sf::RectangleShape midRing(sf::Vector2f(60, 15));
            midRing.setOrigin({30, 7.5f});
            midRing.setPosition({goalX, groundY - 80});
            midRing.setFillColor(totemColor);
            window.draw(midRing);

            sf::RectangleShape upperBody(sf::Vector2f(30, 40));
            upperBody.setOrigin({15, 40});
            upperBody.setPosition({goalX, groundY - 87.5f});
            upperBody.setFillColor(totemColor);
            window.draw(upperBody);

            sf::ConvexShape topCap;
            topCap.setPointCount(4);
            topCap.setPoint(0, sf::Vector2f(goalX - 25, groundY - 127.5f));
            topCap.setPoint(1, sf::Vector2f(goalX + 25, groundY - 127.5f));
            topCap.setPoint(2, sf::Vector2f(goalX + 35, groundY - 142.5f));
            topCap.setPoint(3, sf::Vector2f(goalX - 35, groundY - 142.5f));
            topCap.setFillColor(totemColor);
            window.draw(topCap);

            sf::RectangleShape antenna(sf::Vector2f(4, 30));
            antenna.setOrigin({2, 30});
            antenna.setPosition({goalX, groundY - 142.5f});
            antenna.setFillColor(totemColor);
            window.draw(antenna);

            sf::CircleShape orb(8);
            orb.setOrigin({8, 8});
            orb.setPosition({goalX, groundY - 172.5f});
            orb.setFillColor(totemColor);
            window.draw(orb);

            sf::CircleShape eye(6);
            eye.setOrigin({6, 6});
            eye.setPosition({goalX, groundY - 55});
            eye.setFillColor(accentColor);
            window.draw(eye);

            sf::CircleShape topEye(4);
            topEye.setOrigin({4, 4});
            topEye.setPosition({goalX, groundY - 105});
            topEye.setFillColor(accentColor);
            window.draw(topEye);

            sf::RectangleShape baseLine(sf::Vector2f(40, 4));
            baseLine.setOrigin({20, 2});
            baseLine.setPosition({goalX, groundY - 10});
            baseLine.setFillColor(accentColor);
            window.draw(baseLine);

            std::map<int, int> enemiesAtPos;
            for (const auto& e : game.getEnemies()) {
                if (e->isAlive()) {
                    enemiesAtPos[e->getPos()]++;
                }
            }
            std::set<int> drawnEnemyLabels;

            for (const auto& e : game.getEnemies()) {
                if (!e->isAlive()) continue;
                AnimatedPosition& pos = enemyPositions[e.get()];
                
                sf::CircleShape circle;
                if (dynamic_cast<Boss*>(e.get())) {
                    float r = unitRadius * 1.5f;
                    circle.setRadius(r);
                    circle.setOrigin({r, r});
                    circle.setScale({pos.currentScale, pos.currentScale});
                    circle.setFillColor(sf::Color::Black);
                    circle.setOutlineColor(sf::Color::Red);
                } else {
                    float r = unitRadius;
                    circle.setRadius(r);
                    circle.setOrigin({r, r});
                    circle.setScale({pos.currentScale, pos.currentScale});
                    circle.setFillColor(sf::Color(255, 80, 80));
                    circle.setOutlineColor(sf::Color(150, 30, 30));
                }
                circle.setPosition({pos.currentX, pos.currentY});

                if (dynamic_cast<Boss*>(e.get())) {
                    circle.setOutlineThickness(4);
                } else {
                    circle.setOutlineThickness(3);
                }
                window.draw(circle);

                bool isBoss = (dynamic_cast<Boss*>(e.get()) != nullptr);
                sf::Text typeLabel(font, isBoss ? "Z" : "E", 24);
                typeLabel.setOrigin({typeLabel.getLocalBounds().size.x / 2, typeLabel.getLocalBounds().size.y / 2 + 5});
                typeLabel.setPosition({pos.currentX, pos.currentY});
                if (isBoss) {
                     typeLabel.setFillColor(sf::Color::Red);
                } else {
                     typeLabel.setFillColor(sf::Color::White);
                }
                window.draw(typeLabel);

                if (drawnEnemyLabels.find(e->getPos()) == drawnEnemyLabels.end()) {
                    int count = enemiesAtPos[e->getPos()];
                    
                    if (count > 1) {
                        sf::Text countLabel(font, std::to_string(count), 24);
                        countLabel.setOrigin({countLabel.getLocalBounds().size.x / 2, countLabel.getLocalBounds().size.y / 2});
                        countLabel.setPosition({pos.currentX, pos.currentY - unitRadius - 30.0f}); 
                        countLabel.setFillColor(sf::Color::White);
                        window.draw(countLabel);
                    }
                    
                    drawnEnemyLabels.insert(e->getPos());
                }
            }

            sf::CircleShape armyCircle(unitRadius);
            armyCircle.setOrigin({unitRadius, unitRadius});
            armyCircle.setPosition({armyPos.currentX, armyPos.currentY});
            armyCircle.setFillColor(sf::Color(80, 150, 255));
            armyCircle.setOutlineColor(sf::Color(40, 80, 180));
            armyCircle.setOutlineThickness(4);
            window.draw(armyCircle);

            sf::Text armyTypeLabel(font, "A", 24);
            armyTypeLabel.setOrigin({armyTypeLabel.getLocalBounds().size.x / 2, armyTypeLabel.getLocalBounds().size.y / 2 + 5});
            armyTypeLabel.setPosition({armyPos.currentX, armyPos.currentY});
            armyTypeLabel.setFillColor(sf::Color::White);
            window.draw(armyTypeLabel);

            int livingSoldiers = 0;
            for(const auto& s : game.getArmy().getSoldiers()) {
                if(s->isAlive()) livingSoldiers++;
            }

            sf::Text armyCountLabel(font, std::to_string(livingSoldiers), 24);
            armyCountLabel.setOrigin({armyCountLabel.getLocalBounds().size.x / 2, armyCountLabel.getLocalBounds().size.y / 2});
            armyCountLabel.setPosition({armyPos.currentX, armyPos.currentY - unitRadius - 30.0f});
            armyCountLabel.setFillColor(sf::Color::White);
            window.draw(armyCountLabel);

            if (pataAnimActive) {
                float t = pataAnimTimer / DRUM_ANIM_DURATION;
                float alpha = t < 0.3f ? (t / 0.3f) : ((1.0f - t) / 0.7f);
                alpha = std::clamp(alpha, 0.0f, 1.0f);
                float rotation = -15.0f + (t * 30.0f);
                float scale = 0.8f + (t * 0.2f);

                pataSprite.setRotation(sf::degrees(rotation));
                pataSprite.setScale({scale, scale});
                pataSprite.setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(alpha * 255)));
                window.draw(pataSprite);
            }

            if (ponAnimActive) {
                float t = ponAnimTimer / DRUM_ANIM_DURATION;
                float alpha = t < 0.3f ? (t / 0.3f) : ((1.0f - t) / 0.7f);
                alpha = std::clamp(alpha, 0.0f, 1.0f);
                float rotation = 15.0f - (t * 30.0f);
                float scale = 0.8f + (t * 0.2f);

                ponSprite.setRotation(sf::degrees(rotation));
                ponSprite.setScale({scale, scale});
                ponSprite.setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(alpha * 255)));
                window.draw(ponSprite);
            }

            if (arrowAnim.active) {
                arrowSprite.setPosition({arrowAnim.currentX, arrowAnim.currentY});
                arrowSprite.setScale({0.25f, 0.25f}); 
                window.draw(arrowSprite);
            }

            sf::RectangleShape commandBar(sf::Vector2f(WINDOW_WIDTH, COMMAND_BAR_HEIGHT));
            commandBar.setPosition({0, BATTLEFIELD_HEIGHT});
            commandBar.setFillColor(sf::Color::Black);
            window.draw(commandBar);

            sf::RectangleShape separator(sf::Vector2f(WINDOW_WIDTH, 3));
            separator.setPosition({0, BATTLEFIELD_HEIGHT});
            separator.setFillColor(sf::Color(100, 100, 100));
            window.draw(separator);

            sf::Text moveCmd(font, "Inaintare: PATA PATA PATA PON", 22);
            moveCmd.setPosition({50, BATTLEFIELD_HEIGHT + 30});
            moveCmd.setFillColor(sf::Color::Cyan);
            window.draw(moveCmd);

            sf::Text attackCmd(font, "Atac: PON PON PATA PON", 22);
            attackCmd.setPosition({50, BATTLEFIELD_HEIGHT + 65});
            attackCmd.setFillColor(sf::Color::Red);
            window.draw(attackCmd);

            sf::Text controlsLabel(font, "Controale: A = PATA | D = PON | ESC = Iesire", 18);
            controlsLabel.setPosition({50, BATTLEFIELD_HEIGHT + 110});
            controlsLabel.setFillColor(sf::Color(150, 150, 150));
            window.draw(controlsLabel);

            std::stringstream cmdStream;
            cmdStream << "Secventa curenta: ";
            for (const auto& cmd : game.getCommands().getCommands()) {
                if (cmd == "pa") cmdStream << "PATA ";
                else if (cmd == "po") cmdStream << "PON ";
            }
            
            sf::Text currentSeq(font, cmdStream.str(), 20);
            currentSeq.setPosition({500, BATTLEFIELD_HEIGHT + 30});
            currentSeq.setFillColor(sf::Color::Yellow);
            window.draw(currentSeq);

            if (!game.getLog().empty()) {
                sf::Text lastLog(font, ">>> " + game.getLog().back(), 16);
                lastLog.setPosition({500, BATTLEFIELD_HEIGHT + 100});
                lastLog.setFillColor(sf::Color(200, 255, 200));
                window.draw(lastLog);
            }

            static float bossEventTimer = 0.0f;
            if (game.isBossEventActive()) {
                 bossEventTimer += dt;
                 float fadeDuration = 2.0f;
                 float alpha = 0.0f;
                 
                 if (bossEventTimer < fadeDuration / 2.0f) {
                     alpha = bossEventTimer / (fadeDuration / 2.0f);
                 } else if (bossEventTimer < fadeDuration) {
                     alpha = (fadeDuration - bossEventTimer) / (fadeDuration / 2.0f);
                 } else {
                     game.triggerBossSpawn();
                     bossEventTimer = 0.0f;
                 }
                 
                 if (alpha > 0) {
                     sf::Text bossText(font, "BOSSFIGHT", 100);
                     bossText.setOrigin({bossText.getLocalBounds().size.x / 2, bossText.getLocalBounds().size.y / 2});
                     bossText.setPosition({WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2});
                     bossText.setFillColor(sf::Color(255, 0, 0, static_cast<std::uint8_t>(alpha * 255)));
                     window.draw(bossText);
                 }
            }

            if (game.hasWon()) {
                static bool spaceKeyProcessed = false;
                static bool showStats = false;
                
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
                    if (!spaceKeyProcessed) {
                        showStats = !showStats;
                        spaceKeyProcessed = true;
                    }
                } else {
                    spaceKeyProcessed = false;
                }

                if (showStats) {
                    sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
                    overlay.setFillColor(sf::Color::Black);
                    window.draw(overlay);

                    sf::Text title(font, "STATISTICI", 60);
                    title.setOrigin({title.getLocalBounds().size.x / 2, title.getLocalBounds().size.y / 2});
                    title.setPosition({WINDOW_WIDTH / 2, 100});
                    title.setFillColor(sf::Color::White);
                    window.draw(title);

                    const auto& stats = game.getStats();
                    std::stringstream ss;
                    ss << "Damage Dat: " << stats.getDamageDealt() << "\n"
                       << "Damage Primit: " << stats.getDamageTaken() << "\n"
                       << "Comenzi: " << stats.getCommandsCount() << "\n"
                       << "Pasi: " << stats.getStepsTaken() << "\n"
                       << "Ture: " << stats.getTurns();
                    
                    sf::Text statsText(font, ss.str(), 30);
                    statsText.setOrigin({statsText.getLocalBounds().size.x / 2, statsText.getLocalBounds().size.y / 2});
                    statsText.setPosition({WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2});
                    statsText.setFillColor(sf::Color::White);
                    window.draw(statsText);

                    sf::Text backText(font, "SPACE: Back", 24);
                    backText.setOrigin({backText.getLocalBounds().size.x / 2, backText.getLocalBounds().size.y / 2});
                    backText.setPosition({WINDOW_WIDTH / 2, WINDOW_HEIGHT - 50});
                    backText.setFillColor(sf::Color(150, 150, 150));
                    window.draw(backText);

                } else {
                    sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
                    overlay.setFillColor(sf::Color::Black);
                    window.draw(overlay);

                    sf::Text winText(font, "Nivel Complet", 80);
                    winText.setOrigin({winText.getLocalBounds().size.x / 2, winText.getLocalBounds().size.y / 2});
                    winText.setPosition({WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 50});
                    winText.setFillColor(sf::Color::Green);
                    window.draw(winText);

                    sf::Text retryText(font, "ENTER: Restart\nESC: Iesire", 30);
                    retryText.setOrigin({retryText.getLocalBounds().size.x / 2, retryText.getLocalBounds().size.y / 2});
                    retryText.setPosition({WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 80});
                    retryText.setFillColor(sf::Color::White);
                    window.draw(retryText);
                    
                    sf::Text statsPrompt(font, "SPACE: Stats", 24);
                    statsPrompt.setOrigin({statsPrompt.getLocalBounds().size.x / 2, statsPrompt.getLocalBounds().size.y / 2});
                    statsPrompt.setPosition({WINDOW_WIDTH / 2, WINDOW_HEIGHT - 50});
                    statsPrompt.setFillColor(sf::Color(150, 150, 150));
                    window.draw(statsPrompt);
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter)) {
                     std::vector<std::unique_ptr<Enemy>> emptyEnemies;
                     game = Game(Army(soldiers, 0), std::move(emptyEnemies));
                     armyPos.snapTo(posToX(game.getArmy().getPosition()), fieldY);
                     enemyPositions.clear();
                     for (const auto& e : game.getEnemies()) {
                        AnimatedPosition pos;
                        pos.snapTo(posToX(e->getPos()), fieldY);
                        enemyPositions[e.get()] = pos;
                     }
                     showStats = false;
                }

            } else if (game.hasLost()) {
                sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
                overlay.setFillColor(sf::Color::Black);
                window.draw(overlay);

                sf::Text loseText(font, "Nivel Pierdut", 80);
                loseText.setOrigin({loseText.getLocalBounds().size.x / 2, loseText.getLocalBounds().size.y / 2});
                loseText.setPosition({WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 50});
                loseText.setFillColor(sf::Color::Red);
                window.draw(loseText);

                sf::Text retryText(font, "ENTER: Restart\nESC: Iesire", 30);
                retryText.setOrigin({retryText.getLocalBounds().size.x / 2, retryText.getLocalBounds().size.y / 2});
                retryText.setPosition({WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 80});
                retryText.setFillColor(sf::Color::White);
                window.draw(retryText);

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter)) {
                     soldiers = GameConfig::loadSoldiers("assets/game_config.txt");
                     std::vector<std::unique_ptr<Enemy>> emptyEnemies;
                     game = Game(Army(soldiers, 0), std::move(emptyEnemies));
                     armyPos.snapTo(posToX(game.getArmy().getPosition()), fieldY);
                     enemyPositions.clear();
                     for (const auto& e : game.getEnemies()) {
                        AnimatedPosition pos;
                        pos.snapTo(posToX(e->getPos()), fieldY);
                        enemyPositions[e.get()] = pos;
                     }
                }
            }

            window.display();
        }
    } catch (const GameException& e) {
        std::cerr << "Game Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
