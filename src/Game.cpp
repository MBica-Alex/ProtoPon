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

void Game::processInput(const std::string& input) {
    if (m_won || m_lost || m_bossEventActive) return;
    if (input != "pa" && input != "po") return;
    m_commands.push(input);
    update();
}

void Game::update() {
    if (m_won || m_lost || m_bossEventActive || m_victoryMarchActive) return;
    m_log.clear();

    if (m_beastsSpawned < 3) {
        if (m_enemies.empty() && m_army.getPosition() < m_goal - 5) {
             spawnBeast();
        } else if (rand() % 100 < 10 && m_enemies.size() < 2) {
             spawnBeast();
        }
    } else if (m_beastsDefeated >= 3 && !m_bossSpawned && !m_bossEventActive) {
        if (m_enemies.empty()) {
            m_bossEventActive = true; 
        }
    }

    if (m_commands.matchesMove()) {
        m_stats.addCommand();
        handleMove();
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

    if (!m_army.hasLivingSoldiers()) {
        m_lost = true;
    } else if (m_army.getPosition() >= m_goal && m_bossDefeated() && !m_won && !m_victoryMarchActive) {
         m_victoryMarchActive = true;
    }
}

void Game::finishVictoryMarch() {
    m_victoryMarchActive = false;
    m_won = true;
}

bool Game::m_bossDefeated() const {
    return m_bossSpawned && m_enemies.empty();
}

void Game::spawnBeast() {
    int spawnPos = m_goal - 5;
    if (spawnPos >= GameConstants::MAP_SIZE) spawnPos = GameConstants::MAP_SIZE - 1;
    
    m_enemies.push_back(std::make_unique<Enemy>("Bestia", 20, 2, spawnPos)); 
    m_beastsSpawned++;
    m_log.emplace_back("A APARUT O BESTIE!");
}

void Game::triggerBossSpawn() {
    m_bossEventActive = false;
    spawnBoss();
}

void Game::spawnBoss() {
    if (m_bossSpawned) return;
    int spawnPos = m_goal - 5;
    if (spawnPos >= GameConstants::MAP_SIZE) spawnPos = GameConstants::MAP_SIZE - 1;
    
    m_enemies.push_back(std::make_unique<Boss>("Zigoton General", 50, 2, spawnPos, 3));
    m_bossSpawned = true;
    m_log.emplace_back("GENERALUL ZIGOTON A APARUT!");
}

void Game::render(std::ostream& os) const {
    os << "Inamici:\n";
    for (const auto& e : m_enemies) os << "  " << *e << "\n";
    os << "\nArmata:\n  " << m_army << "\n";
    os << "\nComenzi:";
    for (const auto& c : m_commands.getCommands()) os << " " << c;
    os << "\n";

    if (!m_log.empty()) {
        os << "\n--- Log Batalie ---\n";
        for (const auto& message : m_log) {
            os << ">>> " << message << "\n";
        }
        os << "------------------\n";
    }

    os << "\n=== Camp ===\n";
    for (int pos = 0; pos < GameConstants::MAP_SIZE; ++pos) {
        if (pos == m_army.getPosition()) {
            os << "A";
            continue;
        }
        if (pos == m_goal) {
            os << "G";
            continue;
        }
        int count = 0;
        char c = 'E';
        for (const auto& e : m_enemies) {
            if (!e->isAlive()) continue;
            if (e->getPos() == pos) {
                ++count;
                if (count == 1 && !e->getName().empty())
                    c = static_cast<char>(std::toupper(static_cast<unsigned char>(e->getName()[0])));
            }
        }
        if (count == 0) os << ".";
        else if (count == 1) os << c;
        else if (count < 10) os << static_cast<char>('0' + count);
        else os << "M";
    }
    os << "\n";
    os << "---------------------------\n";


}

bool Game::hasWon() const { return m_won; }
bool Game::hasLost() const { return m_lost; }
const Army& Game::getArmy() const { return m_army; }
const std::vector<std::unique_ptr<Enemy>>& Game::getEnemies() const { return m_enemies; }
const CommandSequence& Game::getCommands() const { return m_commands; }
const std::vector<std::string>& Game::getLog() const { return m_log; }
const GameStats& Game::getStats() const { return m_stats; }
int Game::getGoal() const { return m_goal; }

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

void Game::handleAttack() {
    m_log.emplace_back("ARMATA ATACA!");
    m_army.attackEnemies(m_enemies, m_log, m_stats);
    m_attackTriggered = true;
}

void Game::cleanupDeadEnemies() {
    std::erase_if(m_enemies, [this](const std::unique_ptr<Enemy>& e) { 
        if (!e->isAlive()) {
             if (dynamic_cast<Boss*>(e.get())) {
                 m_log.emplace_back("GENERALUL ZIGOTON A FOST INVINS!");
             } else {
                 m_beastsDefeated++;
                 m_log.emplace_back("BESTIA INVINSA!");
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
        int dist = std::abs(e->getPos() - m_army.getPosition());
        if (dist <= enemyAttackRange) {
            int dmg = e->dealDamage();
            m_army.receiveEnemyAttack(dmg, e->getName(), m_log, m_stats);
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

std::ostream& operator<<(std::ostream& os, const Game& g) {
    g.render(os);
    return os;
}
