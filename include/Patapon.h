#pragma once
#include "Unit.h"
#include "GameConstants.h"
#include "GameException.h"
#include <memory>

class Patapon : public Unit {
public:
    enum class Type { SPEAR, SHIELD, BOW };

    Patapon(Type type, std::string name, int max_hp, int atk, int def);
    ~Patapon() override = default;


    [[nodiscard]] std::unique_ptr<Unit> clone() const override;
    [[nodiscard]] int dealDamage() const override;

    void takeDamage(int dmg) override;

    [[nodiscard]] Type getType() const;
    [[nodiscard]] int getDEF() const;

    static int getTypeRange(Type type);

protected:


private:
    Type m_type;
    int m_def;
};
