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



std::unique_ptr<Unit> Enemy::clone() const {
    return std::make_unique<Enemy>(*this);
}

int Enemy::dealDamage() const {
    return std::max(1, m_atk);
}



int Enemy::getPos() const { return m_pos; }
void Enemy::setPos(int p) { m_pos = p; }

void Enemy::doPrintInfo() const {
}

bool Enemy::isBoss() const {
    return false;
}

std::string Enemy::getDeathMessage() const {
    return "BESTIA A FOST INVINSA!";
}


