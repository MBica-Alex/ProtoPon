#include "Yumipon.h"
#include <iostream>

Yumipon::Yumipon(std::string name, int max_hp, int atk, int def)
    : Patapon(std::move(name), max_hp, atk, def) {}

std::unique_ptr<Unit> Yumipon::clone() const {
    return std::make_unique<Yumipon>(*this);
}

int Yumipon::getRange() const {
    return 3;
}

int Yumipon::dealDamage() const {
    return m_atk + 2;
}


