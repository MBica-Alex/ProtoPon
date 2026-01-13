#include "Patapon.h"
#include <algorithm>

Patapon::Patapon(std::string name, int max_hp, int atk, int def)
    : Unit(std::move(name), max_hp, atk), m_def(def) {
    if (max_hp <= 0) {
        throw InvalidInputException("Patapon HP must be positive");
    }
    if (atk < 0) {
        throw InvalidInputException("Patapon ATK cannot be negative");
    }
}

void Patapon::takeDamage(int dmg) {
    if (dmg < 0) {
        m_hp = std::min(m_max_hp, m_hp - dmg);
    } else {
        int effective = dmg - m_def;
        if (effective < 1) effective = 1;
        m_hp -= effective;
        if (m_hp < 0) m_hp = 0;
    }
}




