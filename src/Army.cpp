#include "Army.h"
#include "GameConstants.h"
#include "GameException.h"
#include <algorithm>

Army::Army(const std::vector<Patapon>& soldiers, int position)
    : m_position(position) {
    if (soldiers.empty()) {
        throw InvalidInputException("Army must have at least one soldier");
    }
    m_soldiers.reserve(soldiers.size());
    for (const auto& s : soldiers) {
        m_soldiers.push_back(s.clone());
    }
}

Army::Army(const Army& other)
    : m_position(other.m_position) {
    m_soldiers.reserve(other.m_soldiers.size());
    for (const auto& p : other.m_soldiers) {
        m_soldiers.push_back(p->clone());
    }
}

Army& Army::operator=(Army other) {
    swap(*this, other);
    return *this;
}

void swap(Army& first, Army& second) noexcept {
    using std::swap;
    swap(first.m_soldiers, second.m_soldiers);
    swap(first.m_position, second.m_position);
}

void Army::moveForward(int steps) {
    if (steps <= 0) return;
    if (!hasLivingSoldiers()) return;
    m_position += steps;
}

void Army::attackEnemies(std::vector<std::unique_ptr<Enemy>>& enemies, std::vector<std::string>& log, GameStats& stats) const {
    if (!hasLivingSoldiers()) return;
    
    std::ranges::sort(enemies, [](const std::unique_ptr<Enemy>& a, const std::unique_ptr<Enemy>& b) {
        return a->getPos() < b->getPos();
    });
    
    for (auto& e : enemies) {
        if (!e->isAlive()) continue;
        int dist = e->getPos() - m_position;
        if (dist < 0) continue;
        int dmg = 0;
        
        for (const auto& p : m_soldiers) {
            if (!p->isAlive()) continue;
            
            if (const auto* patapon = dynamic_cast<Patapon*>(p.get())) {
                int r = Patapon::getTypeRange(patapon->getType());
                if (dist <= r) dmg += p->dealDamage();
            }
        }
        
        if (dmg > 0) {
            int oldHP = e->getHP();
            e->takeDamage(dmg);
            int damageDealt = oldHP - e->getHP();
            stats.addDamageDealt(damageDealt);
            log.push_back("Armata a atacat " + e->getName() + " iar acesta a pierdut " + std::to_string(damageDealt) + " HP!");

            if (e->isAlive()) {
                int retaliate = std::max(1, e->dealDamage() - averageDefense());
                for (const auto& p : m_soldiers) {
                    if (p->isAlive()) {
                        int currentHP = p->getHP();
                        int actualDamage = std::min(currentHP, retaliate);
                        p->takeDamage(retaliate);
                        
                        stats.addDamageTaken(actualDamage);
                        log.push_back(e->getName() + " a contraatacat " + p->getName() + " iar acesta a pierdut " + std::to_string(actualDamage) + " HP!");
                        break;
                    }
                }
            } else {
                log.push_back(e->getName() + " a fost invins!");
            }
            break;
        }
    }
}

void Army::receiveEnemyAttack(int dmg, const std::string& enemyName, std::vector<std::string>& log, GameStats& stats) {
    for (auto& p : m_soldiers) {
        if (p->isAlive()) {
            int oldHP = p->getHP();
            p->takeDamage(dmg);
            int damageTaken = oldHP - p->getHP();
            stats.addDamageTaken(damageTaken);
            log.push_back(enemyName + " a atacat " + p->getName() + " iar acesta a pierdut " + std::to_string(damageTaken) + " HP!");
            if (!p->isAlive()) {
                log.push_back(p->getName() + " a fost invins!");
            }
            break;
        }
    }
}




int Army::averageDefense() const {
    int sum = 0, count = 0;
    for (const auto& p : m_soldiers) {
        if (p->isAlive()) {
            if (const auto* patapon = dynamic_cast<Patapon*>(p.get())) {
                sum += patapon->getDEF();
                ++count;
            }
        }
    }
    return count ? (sum / count) : 0;
}


