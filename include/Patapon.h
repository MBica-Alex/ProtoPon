#pragma once
#include "Unit.h"
#include "GameConstants.h"
#include "GameException.h"
#include <memory>

class Patapon : public Unit {
public:
    Patapon(std::string name, int max_hp, int atk, int def);
    ~Patapon() override = default;

    [[nodiscard]] std::unique_ptr<Unit> clone() const override = 0;
    [[nodiscard]] int dealDamage() const override { return m_atk; }

    void takeDamage(int dmg) override;

    [[nodiscard]] virtual int getRange() const = 0;
    [[nodiscard]] int getDEF() const { return m_def; }
    
protected:
    void doPrintInfo() const override;

protected:
    int m_def;
};
