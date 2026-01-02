#pragma once
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include "Unit.h"
#include "Patapon.h"
#include "Enemy.h"
#include "GameStats.h"

class Army {
public:
    explicit Army(const std::vector<Patapon>& soldiers, int position = 0);
    Army(const Army& other);
    Army& operator=(Army other);
    ~Army() = default;

    friend void swap(Army& first, Army& second) noexcept;

    void moveForward(int steps = 1);
    void attackEnemies(std::vector<std::unique_ptr<Enemy>>& enemies, std::vector<std::string>& log, GameStats& stats) const;
    void receiveEnemyAttack(int dmg, const std::string& enemyName, std::vector<std::string>& log, GameStats& stats);
    [[nodiscard]] bool hasLivingSoldiers() const {
        for (const auto& p : m_soldiers) {
            if (p->isAlive()) return true;
        }
        return false;
    }
    [[nodiscard]] int getPosition() const { return m_position; }
    [[nodiscard]] const std::vector<std::unique_ptr<Unit>>& getSoldiers() const { return m_soldiers; }

    friend std::ostream& operator<<(std::ostream& os, const Army& a);

private:
    std::vector<std::unique_ptr<Unit>> m_soldiers;
    int m_position;

    [[nodiscard]] int averageDefense() const;
};
