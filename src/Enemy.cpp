#include "Enemy.h"
#include <algorithm>
#include <utility>

Enemy::Enemy(std::string name, int hp, int atk, int pos)
    : Unit(std::move(name), hp, atk), m_pos(pos) {
    if (hp <= 0) {
        throw InvalidInputException("Enemy HP must be positive");
    }
    if (pos < 0) {
        throw InvalidInputException("Enemy position cannot be negative");
    }
}

Enemy::Enemy(const Enemy& other)
    : Unit(other), m_pos(other.m_pos) {}

Enemy& Enemy::operator=(Enemy other) {
    swap(*this, other);
    return *this;
}

void swap(Enemy& first, Enemy& second) noexcept {
    using std::swap;
    swap(first.m_name, second.m_name);
    swap(first.m_hp, second.m_hp);
    swap(first.m_max_hp, second.m_max_hp);
    swap(first.m_atk, second.m_atk);
    swap(first.m_pos, second.m_pos);
}

std::unique_ptr<Unit> Enemy::clone() const {
    return std::make_unique<Enemy>(*this);
}

int Enemy::dealDamage() const {
    return std::max(1, m_atk);
}

std::string Enemy::getTypeLabel() const {
    return "Inamic";
}

int Enemy::getPos() const { return m_pos; }
void Enemy::setPos(int p) { m_pos = p; }

void Enemy::renderDetails(std::ostream& os) const {
    os << "[E:" << m_name << " HP:" << m_hp << " ATK:" << m_atk << " POS:" << m_pos << "]";
}
