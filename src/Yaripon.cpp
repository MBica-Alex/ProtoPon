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

void Yaripon::doPrintInfo() const {
    std::cout << "Yaripon [Name=" << m_name << ", HP=" << m_hp << "/" << m_max_hp 
              << ", ATK=" << m_atk << ", DEF=" << m_def << "]" << std::endl;
}
