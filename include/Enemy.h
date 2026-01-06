#pragma once
#include "Unit.h"
#include "GameException.h"
#include <memory>

class Enemy : public Unit {
public:
    Enemy(std::string name, int hp, int atk, int pos);
    ~Enemy() override = default;


    [[nodiscard]] std::unique_ptr<Unit> clone() const override;
    [[nodiscard]] int dealDamage() const override;
    [[nodiscard]] std::string getTypeLabel() const override;

    [[nodiscard]] int getPos() const;
    void setPos(int p);

protected:

    int m_pos;
};
