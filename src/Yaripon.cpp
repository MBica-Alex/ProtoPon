#include "Yaripon.h"
#include <iostream>

Yaripon::Yaripon(std::string name, int max_hp, int atk, int def)
    : Patapon(std::move(name), max_hp, atk, def) {}

std::unique_ptr<Unit> Yaripon::clone() const {
    return std::make_unique<Yaripon>(*this);
}

int Yaripon::getRange() const {
    return 2;
}


