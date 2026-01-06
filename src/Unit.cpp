#include "Unit.h"
#include <algorithm>

Unit::Unit(std::string name, int hp, int atk)
    : m_name(std::move(name)), m_hp(hp), m_max_hp(hp), m_atk(atk) {}



void Unit::takeDamage(int dmg) {
    m_hp -= dmg;
    if (m_hp < 0) m_hp = 0;
}


