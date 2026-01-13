#include "Game.h"
#include "GameException.h"
#include "Boss.h"
#include <algorithm>
#include <cctype>
#include <cmath>

Game::Game(const Army& army, std::vector<std::unique_ptr<Enemy>> enemies)
    : m_army(army), m_enemies(std::move(enemies)), m_won(false), m_lost(false), m_turns(0) {
    m_goal = GameConstants::MAP_SIZE - 1;
}

void Game::update() {
    if (m_won || m_lost || m_bossEventActive || m_victoryMarchActive) return;

    if (m_beastsSpawned < 3) {
        if (m_enemies.empty() && m_army.getPosition() < m_goal - 5) {
             spawnBeast();
        }
    } else if (m_beastsDefeated >= 3 && !m_bossSpawned && !m_bossEventActive) {
        if (m_enemies.empty()) {
            m_bossEventActive = true; 
        }
    }
    
    cleanupDeadEnemies();

    if (!m_army.hasLivingSoldiers()) {
        m_lost = true;
    } else if (m_army.getPosition() >= m_goal && m_bossDefeated() && !m_won && !m_victoryMarchActive) {
         m_victoryMarchActive = true;
    }
}

void Game::processTurn() {
    if (m_won || m_lost || m_bossEventActive || m_victoryMarchActive) return;
    m_log.clear();

    if (m_beastsSpawned < 3) {
        if (rand() % 100 < 10 && m_enemies.size() < 2) {
             spawnBeast();
        }
    }

    if (m_commands.matchesMove()) {
        m_stats.addCommand();
        handleMove();
        m_commands.clear();
    } else if (m_commands.matchesRetreat()) {
        m_stats.addCommand();
        handleRetreat();
        m_commands.clear();
    } else if (m_commands.matchesAttack()) {
        m_stats.addCommand();
        handleAttack();
        m_commands.clear();
    }
    
    cleanupDeadEnemies();
    
    m_turns++;
    m_stats.addTurn();
    if (m_turns % 2 == 0) {
        enemiesAttack();
    } else if (m_turns % 3 == 0) { 
        enemiesAdvance();
    }
    
    cleanupDeadEnemies();
}


void Game::spawnBeast() {
    int spawnPos = m_goal - 5;
    if (spawnPos >= GameConstants::MAP_SIZE) spawnPos = GameConstants::MAP_SIZE - 1;
    
    m_enemies.push_back(std::make_unique<Enemy>("Bestia", 20, 2, spawnPos)); 
    m_beastsSpawned++;
    m_log.emplace_back("A APARUT O BESTIE!");
}

void Game::spawnBoss() {
    if (m_bossSpawned) return;
    int spawnPos = m_goal - 5;
    if (spawnPos >= GameConstants::MAP_SIZE) spawnPos = GameConstants::MAP_SIZE - 1;
    
    m_enemies.push_back(std::make_unique<Boss>("Zigoton General", 50, 2, spawnPos, 3));
    m_bossSpawned = true;
    m_log.emplace_back("GENERALUL ZIGOTON A APARUT!");
}


void Game::handleMove() {
    bool allEnemiesDead = true;
    for (const auto& e : m_enemies) {
        if (e->isAlive()) {
            allEnemiesDead = false;
            break;
        }
    }

    int moveDistance = allEnemiesDead ? 3 : 1;
    int target = m_army.getPosition() + moveDistance;

    if (target > m_goal) {
        target = m_goal;
    }
    
    if (target >= GameConstants::MAP_SIZE) {
        target = GameConstants::MAP_SIZE - 1;
    }

    for (int pos = m_army.getPosition() + 1; pos <= target; ++pos) {
        for (const auto& e : m_enemies) {
            if (!e->isAlive()) continue;
            if (e->getPos() == pos) {
                m_log.emplace_back("MISCARE BLOCATA DE INAMIC!");
                return;
            }
        }
    }

    if (allEnemiesDead) {
        m_log.emplace_back("TOTI INAMICII AU FOST INVINSI! ARMATA INAINTEAZA MAI RAPID!");
    }
    m_log.emplace_back("ARMATA A INAINTAT!");
    m_stats.addSteps(moveDistance);
    m_army.moveForward(moveDistance);
}

void Game::handleRetreat() {
    int currentPos = m_army.getPosition();
    int target = currentPos - 1;

    if (target < 0) {
        m_log.emplace_back("NU POTI MERGE MAI IN SPATE!");
        return;
    }

    m_log.emplace_back("ARMATA S-A RETRAS!");
    m_stats.addSteps(1);
    m_army.moveBackward(1);
}

void Game::handleAttack() {
    m_log.emplace_back("ARMATA ATACA!");
    m_army.attackEnemies(m_enemies, m_log, m_stats);
    m_attackTriggered = true;
}

void Game::cleanupDeadEnemies() {
    std::erase_if(m_enemies, [this](const std::unique_ptr<Enemy>& e) { 
        if (!e->isAlive()) {
             m_log.emplace_back(e->getDeathMessage());
             if (!e->isBoss()) {
                 m_beastsDefeated++;
             }
             return true;
        }
        return false;
    });
}

void Game::enemiesAttack() {
    const int enemyAttackRange = 1;

    for (const auto& e : m_enemies) {
        if (!e->isAlive()) continue;

        Boss* boss = dynamic_cast<Boss*>(e.get());
        if (boss) {
            if (boss->isCharging()) {
                if (boss->getChargeTurns() >= 1) {
                    boss->resetCharge();
                    int dist = std::abs(boss->getPos() - m_army.getPosition());
                    if (dist <= enemyAttackRange) {
                         int dmg = boss->dealDamage() * 2; 
                         m_army.receiveEnemyAttack(dmg, boss->getName(), m_log, m_stats);
                    } else {
                         m_log.emplace_back("GENERALUL ZIGOTON A RATAT ATACUL!");
                    }
                } else {
                    boss->incrementChargeTurns();
                    m_log.emplace_back("GENERALUL ZIGOTON ISI ADUNA PUTERILE!");
                }
            } else {
                int dist = std::abs(boss->getPos() - m_army.getPosition());
                if (dist <= enemyAttackRange) {
                    if (boss->getAttackCount() % 2 == 1) {
                         boss->startCharge();
                         m_log.emplace_back("GENERALUL ZIGOTON PREGATESTE UN ATAC PUTERNIC!");
                         boss->incrementAttackCount();
                    } else {
                         int dmg = boss->dealDamage();
                         m_army.receiveEnemyAttack(dmg, boss->getName(), m_log, m_stats);
                         boss->incrementAttackCount();
                    }
                }
            }
        } else {
            int dist = std::abs(e->getPos() - m_army.getPosition());
            if (dist <= enemyAttackRange) {
                int dmg = e->dealDamage();
                m_army.receiveEnemyAttack(dmg, e->getName(), m_log, m_stats);
            }
        }
    }
}

void Game::enemiesAdvance() {
    for (auto& e : m_enemies) {
        if (!e->isAlive()) continue;
        int armyPos = m_army.getPosition();
        if (e->getPos() > armyPos) {
            int desired = e->getPos() - 1;
            if (desired <= armyPos) continue;
            if (desired >= 0 && desired < GameConstants::MAP_SIZE) {
                e->setPos(desired);
            }
        } else if (e->getPos() < armyPos) {
            int desired = e->getPos() + 1;
            if (desired >= armyPos) continue;
            if (desired >= 0 && desired < GameConstants::MAP_SIZE) {
                e->setPos(desired);
            }
        }
    }
}


