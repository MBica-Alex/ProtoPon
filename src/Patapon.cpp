#include "Patapon.h"
#include <algorithm>
#include <utility>

Patapon::Patapon(Type type, std::string name, int max_hp, int atk, int def)
    : Unit(std::move(name), max_hp, atk), m_type(type), m_def(def) {
    if (max_hp <= 0) {
        throw InvalidInputException("Patapon HP must be positive");
    }
    if (atk < 0) {
        throw InvalidInputException("Patapon ATK cannot be negative");
    }
}

Patapon::Patapon(const Patapon& other)
    : Unit(other), m_type(other.m_type), m_def(other.m_def) {}

Patapon& Patapon::operator=(Patapon other) {
    swap(*this, other);
    return *this;
}

void swap(Patapon& first, Patapon& second) noexcept {
    using std::swap;
    swap(first.m_name, second.m_name);
    swap(first.m_hp, second.m_hp);
    swap(first.m_max_hp, second.m_max_hp);
    swap(first.m_atk, second.m_atk);
    swap(first.m_type, second.m_type);
    swap(first.m_def, second.m_def);
}

std::unique_ptr<Unit> Patapon::clone() const {
    return std::make_unique<Patapon>(*this);
}

int Patapon::dealDamage() const {
    switch (m_type) {
        case Type::BOW: return m_atk + 2;
        case Type::SPEAR: return m_atk;
        case Type::SHIELD: return std::max(0, m_atk - 1);
    }
    return m_atk;
}

std::string Patapon::getTypeLabel() const {
    switch (m_type) {
        case Type::SPEAR:  return GameConstants::LABEL_SPEAR;
        case Type::SHIELD: return GameConstants::LABEL_SHIELD;
        case Type::BOW:    return GameConstants::LABEL_BOW;
    }
    return GameConstants::LABEL_UNKNOWN;
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

Patapon::Type Patapon::getType() const { return m_type; }
int Patapon::getDEF() const { return m_def; }

int Patapon::getTypeRange(Type type) {
    switch (type) {
        case Type::SPEAR: return 2;
        case Type::SHIELD: return 1;
        case Type::BOW: return 3;
    }
    return 1;
}

void Patapon::renderDetails(std::ostream& os) const {
    os << "[" << getTypeLabel() << " HP:" << m_hp << "/" << m_max_hp 
       << " ATK:" << m_atk << " DEF:" << m_def << "]";
}
