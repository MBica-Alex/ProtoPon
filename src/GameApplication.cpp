#include "GameApplication.h"
#include "GameException.h"
#include "GameConfig.h"
#include "Boss.h"
#include <sstream>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <set>

GameApplication::GameApplication() 
    : m_window(sf::VideoMode({static_cast<unsigned>(WINDOW_WIDTH), static_cast<unsigned>(WINDOW_HEIGHT)}), "PROTOPON"),
      m_pataSprite(m_pataTexture),
      m_ponSprite(m_ponTexture),
      m_state(GameState::MENU),
      m_selectedUnits({UnitType::YUMIPON, UnitType::YARIPON, UnitType::TATEPON})
{
    m_window.setFramerateLimit(60);

    sf::Image icon;
    if (icon.loadFromFile("assets/icon.png")) {
        m_window.setIcon(icon.getSize(), icon.getPixelsPtr());
    }

    if (!m_font.openFromFile("assets/pata_font.ttf")) {
        if (!m_font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
            throw ResourceLoadException("Failed to load font: assets/pata_font.ttf or C:/Windows/Fonts/arial.ttf");
        }
    }

    if (!m_pataTexture.loadFromFile("assets/pata.png")) throw ResourceLoadException("assets/pata.png");
    if (!m_ponTexture.loadFromFile("assets/pon.png")) throw ResourceLoadException("assets/pon.png");
    if (!m_yariponTexture.loadFromFile("assets/yaripon.png")) throw ResourceLoadException("assets/yaripon.png");
    if (!m_tateponTexture.loadFromFile("assets/tatepon.png")) throw ResourceLoadException("assets/tatepon.png");
    if (!m_yumiponTexture.loadFromFile("assets/yumipon.png")) throw ResourceLoadException("assets/yumipon.png");

    m_pataponIcons = {&m_tateponTexture, &m_yariponTexture, &m_yumiponTexture};

    m_pataSprite.setTexture(m_pataTexture);
    m_pataSprite.setOrigin(sf::Vector2f(m_pataTexture.getSize()) / 2.0f);
    m_pataSprite.setPosition({80, BATTLEFIELD_HEIGHT / 2});

    m_ponSprite.setTexture(m_ponTexture);
    m_ponSprite.setOrigin(sf::Vector2f(m_ponTexture.getSize()) / 2.0f);
    m_ponSprite.setPosition({WINDOW_WIDTH - 80, BATTLEFIELD_HEIGHT / 2});

    std::vector<std::unique_ptr<Patapon>> soldiers = GameConfig::loadSoldiers("assets/game_config.txt");
    std::vector<std::unique_ptr<Enemy>> initialEnemies;
    m_game = std::make_unique<Game>(Army(std::move(soldiers), 0), std::move(initialEnemies));
    
    m_armyPos = AnimatedPosition();
    m_armyPos.snapTo(posToX(m_game->getArmy().getPosition()), m_fieldY);
    m_enemyPositions.clear();
    for (const auto& e : m_game->getEnemies()) {
        AnimatedPosition pos;
        pos.snapTo(posToX(e->getPos()), m_fieldY);
        m_enemyPositions[e.get()] = pos;
    }
}

float GameApplication::posToX(int pos) const {
    return m_fieldLeft + (static_cast<float>(pos) / (GameConstants::MAP_SIZE - 1)) * m_fieldWidth;
}

void GameApplication::run() {
    sf::Clock clock;
    while (m_window.isOpen()) {
        float dt = clock.restart().asSeconds();
        processEvents();
        update(dt);
        render();
    }
}

void GameApplication::processEvents() {
    while (const auto event = m_window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            m_window.close();
        }

        if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            if (m_state == GameState::MENU) {
                if (keyPressed->code == sf::Keyboard::Key::Left) {
                    m_menuSelectionIndex = (m_menuSelectionIndex - 1 + 3) % 3;
                } else if (keyPressed->code == sf::Keyboard::Key::Right) {
                    m_menuSelectionIndex = (m_menuSelectionIndex + 1) % 3;
                } else if (keyPressed->code == sf::Keyboard::Key::Up) {
                    int currentType = static_cast<int>(m_selectedUnits[m_menuSelectionIndex]);
                    m_selectedUnits[m_menuSelectionIndex] = static_cast<UnitType>((currentType + 1) % 3);
                } else if (keyPressed->code == sf::Keyboard::Key::Down) {
                    int currentType = static_cast<int>(m_selectedUnits[m_menuSelectionIndex]);
                    m_selectedUnits[m_menuSelectionIndex] = static_cast<UnitType>((currentType - 1 + 3) % 3);
                } else if (keyPressed->code == sf::Keyboard::Key::Enter) {
                    std::vector<std::unique_ptr<Patapon>> soldiersConfigFile = GameConfig::loadSoldiers("assets/game_config.txt");
                    
                    std::map<UnitType, const Patapon*> templates;
                    for(const auto& s : soldiersConfigFile) {
                        if (dynamic_cast<Yaripon*>(s.get())) templates[UnitType::YARIPON] = s.get();
                        else if (dynamic_cast<Tatepon*>(s.get())) templates[UnitType::TATEPON] = s.get();
                        else if (dynamic_cast<Yumipon*>(s.get())) templates[UnitType::YUMIPON] = s.get();
                    }

                    std::vector<std::unique_ptr<Patapon>> newSoldiers;

                    auto createFromTemplate = [&](UnitType type) -> std::unique_ptr<Patapon> {
                         const Patapon* tpl = templates[type];
                         if(type == UnitType::YARIPON) return std::make_unique<Yaripon>(tpl->getName(), tpl->getMaxHP(), tpl->getATK(), tpl->getDEF());
                         if(type == UnitType::TATEPON) return std::make_unique<Tatepon>(tpl->getName(), tpl->getMaxHP(), tpl->getATK(), tpl->getDEF());
                         if(type == UnitType::YUMIPON) return std::make_unique<Yumipon>(tpl->getName(), tpl->getMaxHP(), tpl->getATK(), tpl->getDEF());
                         return nullptr;
                    };

                    newSoldiers.push_back(createFromTemplate(m_selectedUnits[2]));
                    newSoldiers.push_back(createFromTemplate(m_selectedUnits[1]));
                    newSoldiers.push_back(createFromTemplate(m_selectedUnits[0]));

                    m_pataponIcons.clear();
                    for(const auto& s : newSoldiers) {
                         if(dynamic_cast<Yaripon*>(s.get())) m_pataponIcons.push_back(&m_yariponTexture);
                         else if(dynamic_cast<Tatepon*>(s.get())) m_pataponIcons.push_back(&m_tateponTexture);
                         else if(dynamic_cast<Yumipon*>(s.get())) m_pataponIcons.push_back(&m_yumiponTexture);
                    }

                    std::vector<std::unique_ptr<Enemy>> initialEnemies;
                    m_game = std::make_unique<Game>(Army(std::move(newSoldiers), 0), std::move(initialEnemies));
                    
                    m_armyPos = AnimatedPosition();
                    m_armyPos.snapTo(posToX(m_game->getArmy().getPosition()), m_fieldY);
                    m_enemyPositions.clear();
                    
                    m_state = GameState::GAME;
                }
            }
            else if (m_state == GameState::GAME) {
                if (m_game->hasWon()) {
                    if (keyPressed->code == sf::Keyboard::Key::Space) {
                        if (!m_spaceKeyProcessed) {
                            m_showStats = !m_showStats;
                            m_spaceKeyProcessed = true;
                        }
                    }
                    if (keyPressed->code == sf::Keyboard::Key::Enter) {
                        m_state = GameState::MENU;
                        m_showStats = false;
                        m_victoryTimer = 0.0f;
                        m_bossEventTimer = 0.0f;
                        m_bossEventAlpha = 0.0f;
                    }
                } else if (m_game->hasLost()) {
                    if (keyPressed->code == sf::Keyboard::Key::Enter) {
                        m_state = GameState::MENU; 
                        m_victoryTimer = 0.0f;
                        m_bossEventTimer = 0.0f;
                        m_bossEventAlpha = 0.0f;
                    }
                } else {
                    if (!m_game->isBossEventActive() && !m_game->isVictoryMarching()) {
                        if (keyPressed->code == sf::Keyboard::Key::A) {
                            m_game->submitCommand("pa");
                            m_pataAnimActive = true;
                            m_pataAnimTimer = 0.0f;
                        } else if (keyPressed->code == sf::Keyboard::Key::D) {
                            m_game->submitCommand("po");
                            m_ponAnimActive = true;
                            m_ponAnimTimer = 0.0f;
                        }
                    }
                }
            }

            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                m_window.close();
            }
        }
    }
    
    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
        m_spaceKeyProcessed = false;
    }
}

void GameApplication::update(float dt) {
    if (m_game->hasWon() || m_game->hasLost()) return;

    if (m_game->isVictoryMarching()) {
            m_victoryTimer += dt;
            
            if (m_victoryTimer < 1.0f) {
                m_armyPos.setTarget(posToX(m_game->getGoal()), m_fieldY);
                m_armyPos.update(dt);
            } 
            else if (m_victoryTimer < 3.0f) {
                float marchProgress = (m_victoryTimer - 1.0f) / 2.0f;
                float startX = posToX(m_game->getGoal());
                float endX = startX + 150.0f;
                
                float currentMarchX = startX + (endX - startX) * marchProgress;
                
                m_armyPos.setTarget(currentMarchX, m_fieldY);
                m_armyPos.update(dt * 0.5f);
            }
            else {
                m_game->finishVictoryMarch();
            }
    } else {
        m_armyPos.setTarget(posToX(m_game->getArmy().getPosition()), m_fieldY);
        m_armyPos.update(dt);
    }

    for (const auto& e : m_game->getEnemies()) {
        if (m_enemyPositions.find(e.get()) == m_enemyPositions.end()) {
            AnimatedPosition pos;
            pos.snapTo(posToX(e->getPos()), m_fieldY);
            pos.startSpawn();
            m_enemyPositions[e.get()] = pos;
        }
        m_enemyPositions[e.get()].setTarget(posToX(e->getPos()), m_fieldY);
        m_enemyPositions[e.get()].update(dt);
    }

    if (m_pataAnimActive) {
        m_pataAnimTimer += dt;
        if (m_pataAnimTimer >= DRUM_ANIM_DURATION) {
            m_pataAnimActive = false;
        }
    }
    
    m_game->update();

    if (m_game->pollAttackTriggered()) {
            float sX = m_armyPos.getCurrentX();
            float sY = m_armyPos.getCurrentY(); 
            float tileWidth = m_fieldWidth / (GameConstants::MAP_SIZE - 1);
            
            int currentPos = m_game->getArmy().getPosition();
            int closestDist = 1000;
            
            for (const auto& e : m_game->getEnemies()) {
                if (!e->isAlive()) continue;
                int dist = e->getPos() - currentPos;
                if (dist >= 0 && dist < closestDist) {
                    closestDist = dist;
                }
            }

            int maxRange = 0;
            for(const auto& s : m_game->getArmy().getSoldiers()) {
                if(s->isAlive()) {
                    maxRange = std::max(maxRange, s->getRange());
                }
            }

            if (closestDist <= 3 && closestDist <= maxRange) {
                 float tX = sX + static_cast<float>(closestDist) * tileWidth;
                 m_arrowAnim.start(sX, sY, tX);
            }
    }
    m_arrowAnim.update(dt);

    if (m_ponAnimActive) {
        m_ponAnimTimer += dt;
        if (m_ponAnimTimer >= DRUM_ANIM_DURATION) {
            m_ponAnimActive = false;
        }
    }
    

    
    if (m_game->isBossEventActive()) {
            m_bossEventTimer += dt;
            float fadeDuration = 2.0f;
            float halfDuration = fadeDuration * 0.5f;
            
            if (m_bossEventTimer < halfDuration) {
                m_bossEventAlpha = m_bossEventTimer / halfDuration;
            } else if (m_bossEventTimer < fadeDuration) {
                m_bossEventAlpha = (fadeDuration - m_bossEventTimer) / halfDuration;
            } else {
                m_game->triggerBossSpawn();
                m_bossEventTimer = 0.0f;
            }
    }
}

void GameApplication::render() {
    if (m_state == GameState::MENU) {
        renderMenu();
        m_window.display();
        return;
    }

    m_window.clear(sf::Color(20, 20, 40));

    sf::RectangleShape sky(sf::Vector2f(WINDOW_WIDTH, BATTLEFIELD_HEIGHT));
    sky.setPosition({0, 0});
    sky.setFillColor(sf::Color(100, 150, 220));
    m_window.draw(sky);

    const float hpCircleRadius = 30.0f;
    const float hpBarWidth = 50.0f;
    const float hpBarHeight = 8.0f;
    float hpStartY = 15.0f;
    float hpSpacing = 70.0f;
    
    const auto& currentSoldiers = m_game->getArmy().getSoldiers();
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
        m_window.draw(circle);

        sf::Sprite iconSprite(*m_pataponIcons[i]);
        float iconScale = (hpCircleRadius * 1.6f) / static_cast<float>(m_pataponIcons[i]->getSize().x);
        iconSprite.setScale({iconScale, iconScale});
        iconSprite.setOrigin(sf::Vector2f(m_pataponIcons[i]->getSize()) / 2.0f);
        iconSprite.setPosition({xPos, yPos});
        m_window.draw(iconSprite);

        float barX = xPos - hpBarWidth / 2;
        float barY = yPos + hpCircleRadius + 5;

        sf::RectangleShape hpBack(sf::Vector2f(hpBarWidth, hpBarHeight));
        hpBack.setPosition({barX, barY});
        hpBack.setFillColor(sf::Color::Black);
        m_window.draw(hpBack);

        float hpPercent = static_cast<float>(currentSoldiers[i]->getHP()) / static_cast<float>(currentSoldiers[i]->getMaxHP());
        hpPercent = std::clamp(hpPercent, 0.0f, 1.0f);

        std::uint8_t r = static_cast<std::uint8_t>((1.0f - hpPercent) * 255);
        std::uint8_t g = static_cast<std::uint8_t>(hpPercent * 255);
        sf::Color hpColor(r, g, 0);

        sf::RectangleShape hpBar(sf::Vector2f(hpBarWidth * hpPercent, hpBarHeight));
        hpBar.setPosition({barX, barY});
        hpBar.setFillColor(hpColor);
        m_window.draw(hpBar);
    }

    float goalX = posToX(m_game->getGoal());
    
    sf::Color totemColor = sf::Color::Black;
    sf::Color accentColor = (m_game->hasWon() || m_game->isVictoryMarching()) ? sf::Color::Green : sf::Color::Red;
    
    float groundY = BATTLEFIELD_HEIGHT;

    sf::ConvexShape base;
    base.setPointCount(4);
    base.setPoint(0, sf::Vector2f(goalX - 30, groundY));
    base.setPoint(1, sf::Vector2f(goalX + 30, groundY));
    base.setPoint(2, sf::Vector2f(goalX + 20, groundY - 30));
    base.setPoint(3, sf::Vector2f(goalX - 20, groundY - 30));
    base.setFillColor(totemColor);
    m_window.draw(base);

    sf::RectangleShape lowerBody(sf::Vector2f(40, 50));
    lowerBody.setOrigin({20, 50});
    lowerBody.setPosition({goalX, groundY - 30});
    lowerBody.setFillColor(totemColor);
    m_window.draw(lowerBody);

    sf::RectangleShape midRing(sf::Vector2f(60, 15));
    midRing.setOrigin({30, 7.5f});
    midRing.setPosition({goalX, groundY - 80});
    midRing.setFillColor(totemColor);
    m_window.draw(midRing);

    sf::RectangleShape upperBody(sf::Vector2f(30, 40));
    upperBody.setOrigin({15, 40});
    upperBody.setPosition({goalX, groundY - 87.5f});
    upperBody.setFillColor(totemColor);
    m_window.draw(upperBody);

    sf::ConvexShape topCap;
    topCap.setPointCount(4);
    topCap.setPoint(0, sf::Vector2f(goalX - 25, groundY - 127.5f));
    topCap.setPoint(1, sf::Vector2f(goalX + 25, groundY - 127.5f));
    topCap.setPoint(2, sf::Vector2f(goalX + 35, groundY - 142.5f));
    topCap.setPoint(3, sf::Vector2f(goalX - 35, groundY - 142.5f));
    topCap.setFillColor(totemColor);
    m_window.draw(topCap);

    sf::RectangleShape antenna(sf::Vector2f(4, 30));
    antenna.setOrigin({2, 30});
    antenna.setPosition({goalX, groundY - 142.5f});
    antenna.setFillColor(totemColor);
    m_window.draw(antenna);

    sf::CircleShape orb(8);
    orb.setOrigin({8, 8});
    orb.setPosition({goalX, groundY - 172.5f});
    orb.setFillColor(totemColor);
    m_window.draw(orb);

    sf::CircleShape eye(6);
    eye.setOrigin({6, 6});
    eye.setPosition({goalX, groundY - 55});
    eye.setFillColor(accentColor);
    m_window.draw(eye);

    sf::CircleShape topEye(4);
    topEye.setOrigin({4, 4});
    topEye.setPosition({goalX, groundY - 105});
    topEye.setFillColor(accentColor);
    m_window.draw(topEye);

    sf::RectangleShape baseLine(sf::Vector2f(40, 4));
    baseLine.setOrigin({20, 2});
    baseLine.setPosition({goalX, groundY - 10});
    baseLine.setFillColor(accentColor);
    m_window.draw(baseLine);

    std::map<int, int> enemiesAtPos;
    for (const auto& e : m_game->getEnemies()) {
        if (e->isAlive()) {
            enemiesAtPos[e->getPos()]++;
        }
    }
    std::set<int> drawnEnemyLabels;

    for (const auto& e : m_game->getEnemies()) {
        if (!e->isAlive()) continue;
        const AnimatedPosition& pos = m_enemyPositions[e.get()];
        
        sf::CircleShape circle;
        if (dynamic_cast<Boss*>(e.get())) {
            float r = m_unitRadius * 1.5f;
            circle.setRadius(r);
            circle.setOrigin({r, r});
            circle.setScale({pos.getCurrentScale(), pos.getCurrentScale()});
            circle.setFillColor(sf::Color::Black);
            circle.setOutlineColor(sf::Color::Red);
        } else {
            float r = m_unitRadius;
            circle.setRadius(r);
            circle.setOrigin({r, r});
            circle.setScale({pos.getCurrentScale(), pos.getCurrentScale()});
            circle.setFillColor(sf::Color(255, 80, 80));
            circle.setOutlineColor(sf::Color(150, 30, 30));
        }
        circle.setPosition({pos.getCurrentX(), pos.getCurrentY()});

        if (dynamic_cast<Boss*>(e.get())) {
            circle.setOutlineThickness(4);
        } else {
            circle.setOutlineThickness(3);
        }
        m_window.draw(circle);

        bool isBoss = (dynamic_cast<Boss*>(e.get()) != nullptr);
        sf::Text typeLabel(m_font, isBoss ? "Z" : "E", 24);
        typeLabel.setOrigin({typeLabel.getLocalBounds().size.x / 2, typeLabel.getLocalBounds().size.y / 2 + 5});
        typeLabel.setPosition({pos.getCurrentX(), pos.getCurrentY()});
        if (isBoss) {
                typeLabel.setFillColor(sf::Color::Red);
        } else {
                typeLabel.setFillColor(sf::Color::White);
        }
        m_window.draw(typeLabel);

        if (drawnEnemyLabels.find(e->getPos()) == drawnEnemyLabels.end()) {
            int count = enemiesAtPos[e->getPos()];
            
            if (count > 1) {
                sf::Text countLabel(m_font, std::to_string(count), 24);
                countLabel.setOrigin({countLabel.getLocalBounds().size.x / 2, countLabel.getLocalBounds().size.y / 2});
                countLabel.setPosition({pos.getCurrentX(), pos.getCurrentY() - m_unitRadius - 30.0f}); 
                countLabel.setFillColor(sf::Color::White);
                m_window.draw(countLabel);
            }
            
            drawnEnemyLabels.insert(e->getPos());
        }
    }

    sf::CircleShape armyCircle(m_unitRadius);
    armyCircle.setOrigin({m_unitRadius, m_unitRadius});
    armyCircle.setPosition({m_armyPos.getCurrentX(), m_armyPos.getCurrentY()});
    armyCircle.setFillColor(sf::Color(80, 150, 255));
    armyCircle.setOutlineColor(sf::Color(40, 80, 180));
    armyCircle.setOutlineThickness(4);
    m_window.draw(armyCircle);

    sf::Text armyTypeLabel(m_font, "A", 24);
    armyTypeLabel.setOrigin({armyTypeLabel.getLocalBounds().size.x / 2, armyTypeLabel.getLocalBounds().size.y / 2 + 5});
    armyTypeLabel.setPosition({m_armyPos.getCurrentX(), m_armyPos.getCurrentY()});
    armyTypeLabel.setFillColor(sf::Color::White);
    m_window.draw(armyTypeLabel);

    int livingSoldiers = 0;
    for(const auto& s : m_game->getArmy().getSoldiers()) {
        if(s->isAlive()) livingSoldiers++;
    }

    sf::Text armyCountLabel(m_font, std::to_string(livingSoldiers), 24);
    armyCountLabel.setOrigin({armyCountLabel.getLocalBounds().size.x / 2, armyCountLabel.getLocalBounds().size.y / 2});
    armyCountLabel.setPosition({m_armyPos.getCurrentX(), m_armyPos.getCurrentY() - m_unitRadius - 30.0f});
    armyCountLabel.setFillColor(sf::Color::White);
    m_window.draw(armyCountLabel);

    if (m_pataAnimActive) {
        float t = m_pataAnimTimer / DRUM_ANIM_DURATION;
        float alpha = t < 0.3f ? (t / 0.3f) : ((1.0f - t) / 0.7f);
        alpha = std::clamp(alpha, 0.0f, 1.0f);
        float rotation = -15.0f + (t * 30.0f);
        float scale = 0.8f + (t * 0.2f);

        m_pataSprite.setRotation(sf::degrees(rotation));
        m_pataSprite.setScale({scale, scale});
        m_pataSprite.setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(alpha * 255)));
        m_window.draw(m_pataSprite);
    }

    if (m_ponAnimActive) {
        float t = m_ponAnimTimer / DRUM_ANIM_DURATION;
        float alpha = t < 0.3f ? (t / 0.3f) : ((1.0f - t) / 0.7f);
        alpha = std::clamp(alpha, 0.0f, 1.0f);
        float rotation = 15.0f - (t * 30.0f);
        float scale = 0.8f + (t * 0.2f);

        m_ponSprite.setRotation(sf::degrees(rotation));
        m_ponSprite.setScale({scale, scale});
        m_ponSprite.setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(alpha * 255)));
        m_window.draw(m_ponSprite);
    }

    if (m_arrowAnim.isActive()) {
        sf::ConvexShape arrowShape;
        arrowShape.setPointCount(7);
        arrowShape.setPoint(0, sf::Vector2f(-25.0f, -2.0f));
        arrowShape.setPoint(1, sf::Vector2f(10.0f, -2.0f));
        arrowShape.setPoint(2, sf::Vector2f(10.0f, -8.0f));
        arrowShape.setPoint(3, sf::Vector2f(35.0f, 0.0f));
        arrowShape.setPoint(4, sf::Vector2f(10.0f, 8.0f));
        arrowShape.setPoint(5, sf::Vector2f(10.0f, 2.0f));
        arrowShape.setPoint(6, sf::Vector2f(-25.0f, 2.0f));

        arrowShape.setFillColor(sf::Color::Black);
        arrowShape.setOutlineColor(sf::Color::White);
        arrowShape.setOutlineThickness(1.0f);
        
        arrowShape.setPosition({m_arrowAnim.getCurrentX(), m_arrowAnim.getCurrentY()});
        arrowShape.setRotation(sf::degrees(m_arrowAnim.getRotation()));

        m_window.draw(arrowShape);
    }

    sf::RectangleShape commandBar(sf::Vector2f(WINDOW_WIDTH, COMMAND_BAR_HEIGHT));
    commandBar.setPosition({0, BATTLEFIELD_HEIGHT});
    commandBar.setFillColor(sf::Color::Black);
    m_window.draw(commandBar);

    sf::RectangleShape separator(sf::Vector2f(WINDOW_WIDTH, 3));
    separator.setPosition({0, BATTLEFIELD_HEIGHT});
    separator.setFillColor(sf::Color(100, 100, 100));
    m_window.draw(separator);

    sf::Text moveCmd(m_font, "Inaintare: PATA PATA PATA PON", 22);
    moveCmd.setPosition({50, BATTLEFIELD_HEIGHT + 30});
    moveCmd.setFillColor(sf::Color::Cyan);
    m_window.draw(moveCmd);

    sf::Text attackCmd(m_font, "Atac: PON PON PATA PON", 22);
    attackCmd.setPosition({50, BATTLEFIELD_HEIGHT + 65});
    attackCmd.setFillColor(sf::Color::Red);
    m_window.draw(attackCmd);

    sf::Text controlsLabel(m_font, "Controale: A = PATA | D = PON | ESC = Iesire", 18);
    controlsLabel.setPosition({50, BATTLEFIELD_HEIGHT + 110});
    controlsLabel.setFillColor(sf::Color(150, 150, 150));
    m_window.draw(controlsLabel);

    std::stringstream cmdStream;
    cmdStream << "Secventa curenta: ";
    for (const auto& cmd : m_game->getCommands().getCommands()) {
        if (cmd == "pa") cmdStream << "PATA ";
        else if (cmd == "po") cmdStream << "PON ";
    }
    
    sf::Text currentSeq(m_font, cmdStream.str(), 20);
    currentSeq.setPosition({500, BATTLEFIELD_HEIGHT + 30});
    currentSeq.setFillColor(sf::Color::Yellow);
    m_window.draw(currentSeq);

    if (!m_game->getLog().empty()) {
        sf::Text lastLog(m_font, ">>> " + m_game->getLog().back(), 16);
        lastLog.setPosition({500, BATTLEFIELD_HEIGHT + 100});
        lastLog.setFillColor(sf::Color(200, 255, 200));
        m_window.draw(lastLog);
    }

    if (m_game->isBossEventActive() && m_bossEventAlpha > 0) {
            sf::Text bossText(m_font, "BOSSFIGHT", 100);
            bossText.setOrigin({bossText.getLocalBounds().size.x / 2, bossText.getLocalBounds().size.y / 2});
            bossText.setPosition({WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2});
            bossText.setFillColor(sf::Color(255, 0, 0, static_cast<std::uint8_t>(m_bossEventAlpha * 255)));
            m_window.draw(bossText);
    }

    if (m_game->hasWon()) {
        if (m_showStats) renderStats();
        else renderWinScreen();
    } else if (m_game->hasLost()) {
        renderLoseScreen();
    }

    m_window.display();
}

void GameApplication::renderStats() {
    sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color::Black);
    m_window.draw(overlay);

    sf::Text title(m_font, "STATISTICI", 60);
    title.setOrigin({title.getLocalBounds().size.x / 2, title.getLocalBounds().size.y / 2});
    title.setPosition({WINDOW_WIDTH / 2, 100});
    title.setFillColor(sf::Color::White);
    m_window.draw(title);

    const auto& stats = m_game->getStats();
    std::stringstream ss;
    ss << "Damage Dat: " << stats.getDamageDealt() << "\n"
       << "Damage Primit: " << stats.getDamageTaken() << "\n"
       << "Comenzi: " << stats.getCommandsCount() << "\n"
       << "Pasi: " << stats.getStepsTaken() << "\n"
       << "Ture: " << stats.getTurns();
    
    sf::Text statsText(m_font, ss.str(), 30);
    statsText.setOrigin({statsText.getLocalBounds().size.x / 2, statsText.getLocalBounds().size.y / 2});
    statsText.setPosition({WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2});
    statsText.setFillColor(sf::Color::White);
    m_window.draw(statsText);

    sf::Text backText(m_font, "SPACE: Back", 24);
    backText.setOrigin({backText.getLocalBounds().size.x / 2, backText.getLocalBounds().size.y / 2});
    backText.setPosition({WINDOW_WIDTH / 2, WINDOW_HEIGHT - 50});
    backText.setFillColor(sf::Color(150, 150, 150));
    m_window.draw(backText);
}

void GameApplication::renderWinScreen() {
    sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color::Black);
    m_window.draw(overlay);

    sf::Text winText(m_font, "Nivel Complet", 80);
    winText.setOrigin({winText.getLocalBounds().size.x / 2, winText.getLocalBounds().size.y / 2});
    winText.setPosition({WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 50});
    winText.setFillColor(sf::Color::Green);
    m_window.draw(winText);

    sf::Text retryText(m_font, "ENTER: Restart\nESC: Iesire", 30);
    retryText.setOrigin({retryText.getLocalBounds().size.x / 2, retryText.getLocalBounds().size.y / 2});
    retryText.setPosition({WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 80});
    retryText.setFillColor(sf::Color::White);
    m_window.draw(retryText);
    
    sf::Text statsPrompt(m_font, "SPACE: Stats", 24);
    statsPrompt.setOrigin({statsPrompt.getLocalBounds().size.x / 2, statsPrompt.getLocalBounds().size.y / 2});
    statsPrompt.setPosition({WINDOW_WIDTH / 2, WINDOW_HEIGHT - 50});
    statsPrompt.setFillColor(sf::Color(150, 150, 150));
    m_window.draw(statsPrompt);
}

void GameApplication::renderLoseScreen() {
    sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color::Black);
    m_window.draw(overlay);

    sf::Text loseText(m_font, "Nivel Pierdut", 80);
    loseText.setOrigin({loseText.getLocalBounds().size.x / 2, loseText.getLocalBounds().size.y / 2});
    loseText.setPosition({WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 50});
    loseText.setFillColor(sf::Color::Red);
    m_window.draw(loseText);

    sf::Text retryText(m_font, "ENTER: Restart\nESC: Iesire", 30);
    retryText.setOrigin({retryText.getLocalBounds().size.x / 2, retryText.getLocalBounds().size.y / 2});
    retryText.setPosition({WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 80});
    retryText.setFillColor(sf::Color::White);
    m_window.draw(retryText);
}

const sf::Texture& GameApplication::getUnitTexture(UnitType type) const {
    switch (type) {
        case UnitType::YARIPON: return m_yariponTexture;
        case UnitType::TATEPON: return m_tateponTexture;
        case UnitType::YUMIPON: return m_yumiponTexture;
        default: return m_yariponTexture;
    }
}

void GameApplication::renderMenu() {
    m_window.clear(sf::Color(10, 10, 20));

    sf::Text title(m_font, "SELECTEAZA ARMATA", 50);
    title.setOrigin({title.getLocalBounds().size.x / 2, title.getLocalBounds().size.y / 2});
    title.setPosition({WINDOW_WIDTH / 2, 80});
    title.setFillColor(sf::Color::White);
    m_window.draw(title);

    sf::Text instr(m_font, "Sageata STANGA/DREAPTA: Alege Slot\nSageata SUS/JOS: Schimba Unitate\nENTER: Start Lupta", 20);
    instr.setOrigin({instr.getLocalBounds().size.x / 2, instr.getLocalBounds().size.y / 2});
    instr.setPosition({WINDOW_WIDTH / 2, 160});
    instr.setFillColor(sf::Color(150, 150, 150));
    m_window.draw(instr);

    float startX = WINDOW_WIDTH / 2 - 250;
    float slotY = WINDOW_HEIGHT / 2;
    float slotSpacing = 250.0f;

    std::vector<std::string> slotNames = { "SPATE", "MIJLOC", "FATA" };
    
    for (int i = 0; i < 3; ++i) {
        float x = startX + i * slotSpacing;
        
        if (m_menuSelectionIndex == i) {
            sf::RectangleShape highlight(sf::Vector2f(200, 300));
            highlight.setOrigin({100, 150});
            highlight.setPosition({x, slotY});
            highlight.setFillColor(sf::Color(50, 50, 100));
            highlight.setOutlineColor(sf::Color::Cyan);
            highlight.setOutlineThickness(3);
            m_window.draw(highlight);
        }

        sf::Text slotName(m_font, slotNames[i], 24);
        slotName.setOrigin({slotName.getLocalBounds().size.x / 2, slotName.getLocalBounds().size.y / 2});
        slotName.setPosition({x, slotY - 120});
        m_window.draw(slotName);

        UnitType type = m_selectedUnits[i];
        sf::Sprite icon(getUnitTexture(type));
        float targetSize = 100.0f;
        float scale = targetSize / icon.getLocalBounds().size.x;
        icon.setScale({scale, scale});
        icon.setOrigin({icon.getLocalBounds().size.x / 2, icon.getLocalBounds().size.y / 2});
        icon.setPosition({x, slotY});
        m_window.draw(icon);
        
        std::string unitName = "Unknown";
        if (type == UnitType::YARIPON) unitName = "YARIPON (Sulita)";
        else if (type == UnitType::TATEPON) unitName = "TATEPON (Scut)";
        else if (type == UnitType::YUMIPON) unitName = "YUMIPON (Arc)";

        sf::Text uName(m_font, unitName, 20);
        uName.setOrigin({uName.getLocalBounds().size.x / 2, uName.getLocalBounds().size.y / 2});
        uName.setPosition({x, slotY + 100});
        uName.setFillColor(sf::Color::Yellow);
        m_window.draw(uName);
    }
}
