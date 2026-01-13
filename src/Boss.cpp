#include "Boss.h"
#include <algorithm>
#include <utility>

Boss::Boss(std::string name, int hp, int atk, int pos, int bonusDamage)
    : Enemy(std::move(name), hp, atk, pos), m_bonusDamage(bonusDamage) {
    if (bonusDamage < 0) {
        throw InvalidInputException("Boss bonus damage cannot be negative");
    }
}



std::unique_ptr<Unit> Boss::clone() const {
    return std::make_unique<Boss>(*this);
}

int Boss::dealDamage() const {
    return std::max(1, m_atk) + m_bonusDamage;
}



bool Boss::isBoss() const {
    return true;
}

std::string Boss::getDeathMessage() const {
    return "GENERALUL ZIGOTON A FOST INVINS!";
}

