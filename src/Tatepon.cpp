#include "Tatepon.h"
#include <iostream>
#include <algorithm>

Tatepon::Tatepon(std::string name, int max_hp, int atk, int def)
    : Patapon(std::move(name), max_hp, atk, def) {}

std::unique_ptr<Unit> Tatepon::clone() const {
    return std::make_unique<Tatepon>(*this);
}

int Tatepon::getRange() const {
    return 1;
}

int Tatepon::dealDamage() const {
    return std::max(0, m_atk - 1);
}


