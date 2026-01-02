#include "Boss.h"
#include <algorithm>
#include <utility>

Boss::Boss(std::string name, int hp, int atk, int pos, int bonusDamage)
    : Enemy(std::move(name), hp, atk, pos), m_bonusDamage(bonusDamage) {
    if (bonusDamage < 0) {
        throw InvalidInputException("Boss bonus damage cannot be negative");
    }
}

Boss::Boss(const Boss& other)
    : Enemy(other), m_bonusDamage(other.m_bonusDamage) {}

Boss& Boss::operator=(Boss other) {
    swap(*this, other);
    return *this;
}

void swap(Boss& first, Boss& second) noexcept {
    using std::swap;
    swap(first.m_name, second.m_name);
    swap(first.m_hp, second.m_hp);
    swap(first.m_max_hp, second.m_max_hp);
    swap(first.m_atk, second.m_atk);
    swap(first.m_pos, second.m_pos);
    swap(first.m_bonusDamage, second.m_bonusDamage);
}

std::unique_ptr<Unit> Boss::clone() const {
    return std::make_unique<Boss>(*this);
}

int Boss::dealDamage() const {
    return std::max(1, m_atk) + m_bonusDamage;
}

std::string Boss::getTypeLabel() const {
    return "Boss";
}



void Boss::renderDetails(std::ostream& os) const {
    os << "[BOSS:" << m_name << " HP:" << m_hp << " ATK:" << m_atk 
       << "+" << m_bonusDamage << " POS:" << m_pos << "]";
}
