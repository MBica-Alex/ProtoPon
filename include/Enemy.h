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


    [[nodiscard]] int getPos() const;

    void setPos(int p);

    [[nodiscard]] virtual bool isBoss() const;
    [[nodiscard]] virtual std::string getDeathMessage() const;

protected:
    void doPrintInfo() const override;

protected:

    int m_pos;
};
