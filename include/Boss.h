#pragma once
#include "Enemy.h"
#include <memory>

class Boss : public Enemy {
public:
    Boss(std::string name, int hp, int atk, int pos, int bonusDamage);
    ~Boss() override = default;


    [[nodiscard]] std::unique_ptr<Unit> clone() const override;
    
    [[nodiscard]] bool isBoss() const override;
    [[nodiscard]] std::string getDeathMessage() const override;

    [[nodiscard]] int dealDamage() const override;

    [[nodiscard]] bool isCharging() const;
    void startCharge();
    void resetCharge();
    void incrementChargeTurns();
    [[nodiscard]] int getChargeTurns() const;
    void incrementAttackCount();
    [[nodiscard]] int getAttackCount() const;

private:
    int m_bonusDamage;
    bool m_isCharging;
    int m_chargeTurns;
    int m_attackCount;
};
