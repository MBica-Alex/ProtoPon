#pragma once
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include "Army.h"
#include "Enemy.h"
#include "CommandSequence.h"
#include "GameStats.h"
#include "GameConstants.h"

class Game {
public:
    Game(const Army& army, std::vector<std::unique_ptr<Enemy>> enemies);

    void processInput(const std::string& input);
    void update();
    void render(std::ostream& os) const;

    [[nodiscard]] bool hasWon() const;
    [[nodiscard]] bool hasLost() const;
    [[nodiscard]] const Army& getArmy() const;
    [[nodiscard]] const std::vector<std::unique_ptr<Enemy>>& getEnemies() const;
    [[nodiscard]] const CommandSequence& getCommands() const;
    [[nodiscard]] const std::vector<std::string>& getLog() const;
    [[nodiscard]] const GameStats& getStats() const;
    [[nodiscard]] int getGoal() const;

    friend std::ostream& operator<<(std::ostream& os, const Game& g);

private:
    Army m_army;
    std::vector<std::unique_ptr<Enemy>> m_enemies;
    CommandSequence m_commands;
    std::vector<std::string> m_log;
    GameStats m_stats;
    bool m_won;
    bool m_lost;
    int m_turns;
    int m_goal;

    void handleMove();
    void handleAttack();
    void cleanupDeadEnemies();
    void enemiesAttack();
    void enemiesAdvance();
    void spawnBeast();
    void spawnBoss();

public:
    [[nodiscard]] bool isBossEventActive() const { return m_bossEventActive; }
    [[nodiscard]] bool isVictoryMarching() const { return m_victoryMarchActive; }
    void triggerBossSpawn();
    void finishVictoryMarch();

    bool pollAttackTriggered() {
        bool temp = m_attackTriggered;
        m_attackTriggered = false;
        return temp;
    }

private:
    int m_beastsSpawned = 0;
    int m_beastsDefeated = 0;
    bool m_bossSpawned = false;
    bool m_bossEventActive = false;
    bool m_victoryMarchActive = false;
    
    bool m_bossDefeated() const;

private:
    bool m_attackTriggered = false;
};
