#pragma once
#include "Enemy.h"
#include <memory>

class Boss : public Enemy {
public:
    Boss(std::string name, int hp, int atk, int pos, int bonusDamage);
    ~Boss() override = default;


    [[nodiscard]] std::unique_ptr<Unit> clone() const override;
    [[nodiscard]] int dealDamage() const override;
    [[nodiscard]] std::string getTypeLabel() const override;

protected:


private:
    int m_bonusDamage;
};
