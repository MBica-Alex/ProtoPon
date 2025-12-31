#include "Unit.h"
#include <algorithm>

Unit::Unit(std::string name, int hp, int atk)
    : m_name(std::move(name)), m_hp(hp), m_max_hp(hp), m_atk(atk) {}

Unit::Unit(const Unit& other)
    : m_name(other.m_name), m_hp(other.m_hp), m_max_hp(other.m_max_hp), m_atk(other.m_atk) {}

Unit& Unit::operator=(const Unit& other) {
    if (this != &other) {
        m_name = other.m_name;
        m_hp = other.m_hp;
        m_max_hp = other.m_max_hp;
        m_atk = other.m_atk;
    }
    return *this;
}

void Unit::takeDamage(int dmg) {
    m_hp -= dmg;
    if (m_hp < 0) m_hp = 0;
}

void Unit::print(std::ostream& os) const {
    renderDetails(os);
}

std::ostream& operator<<(std::ostream& os, const Unit& unit) {
    unit.print(os);
    return os;
}

const std::string& Unit::getName() const { return m_name; }
int Unit::getHP() const { return m_hp; }
int Unit::getMaxHP() const { return m_max_hp; }
int Unit::getATK() const { return m_atk; }
bool Unit::isAlive() const { return m_hp > 0; }
