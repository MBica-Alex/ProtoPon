#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>
#include <cctype>
#include <cmath>

namespace GameConstants {
    constexpr int MAP_SIZE = 15;
    constexpr const char* LABEL_SPEAR = "Sulita";
    constexpr const char* LABEL_SHIELD = "Scut";
    constexpr const char* LABEL_BOW = "Arc";
    constexpr const char* LABEL_UNKNOWN = "Necunoscut";
}

class Patapon {
public:
    enum class Type { SPEAR, SHIELD, BOW };
    Patapon(Type type, std::string name, int max_hp, int atk, int def)
        : m_type(type), m_name(std::move(name)), m_hp(max_hp), m_max_hp(max_hp), m_atk(atk), m_def(def) {}
    Patapon(const Patapon &other)
        : m_type(other.m_type), m_name(other.m_name), m_hp(other.m_hp), m_max_hp(other.m_max_hp), m_atk(other.m_atk), m_def(other.m_def) {}
    Patapon& operator=(const Patapon &other) {
        if (this != &other) {
            m_type = other.m_type;
            m_name = other.m_name;
            m_hp = other.m_hp;
            m_max_hp = other.m_max_hp;
            m_atk = other.m_atk;
            m_def = other.m_def;
        }
        return *this;
    }
    ~Patapon() = default;
    Type getType() const { return m_type; }
    int getHP() const { return m_hp; }
    int getMaxHP() const { return m_max_hp; }
    int getATK() const { return m_atk; }
    int getDEF() const { return m_def; }
    std::string getTypeLabel() const {
        switch (m_type) {
            case Type::SPEAR:  return GameConstants::LABEL_SPEAR;
            case Type::SHIELD: return GameConstants::LABEL_SHIELD;
            case Type::BOW:    return GameConstants::LABEL_BOW;
        }
        return GameConstants::LABEL_UNKNOWN;
    }
    bool isAlive() const { return m_hp > 0; }
    void takeDamage(int dmg) {
        if (dmg < 0) {
            m_hp = std::min(m_max_hp, m_hp - dmg);
        } else {
            int effective = dmg - m_def;
            if (effective < 1) effective = 1;
            m_hp -= effective;
            if (m_hp < 0) m_hp = 0;
        }
    }
    int dealDamage() const {
        switch (m_type) {
            case Type::BOW: return m_atk + 2;
            case Type::SPEAR: return m_atk;
            case Type::SHIELD: return std::max(0, m_atk - 1);
        }
        return m_atk;
    }
    friend std::ostream& operator<<(std::ostream &os, const Patapon &s) {
        os << "[" << s.getTypeLabel() << " HP:" << s.m_hp << "/" << s.m_max_hp << " ATK:" << s.m_atk << " DEF:" << s.m_def << "]";
        return os;
    }
private:
    Type m_type;
    std::string m_name;
    int m_hp;
    int m_max_hp;
    int m_atk;
    int m_def;
};

class Enemy {
public:
    Enemy(std::string name, int hp, int atk, int pos)
        : m_name(std::move(name)), m_hp(hp), m_atk(atk), m_pos(pos) {}
    const std::string& getName() const { return m_name; }
    int getHP() const { return m_hp; }
    int getATK() const { return m_atk; }
    int getPos() const { return m_pos; }
    void setPos(int p) { m_pos = p; }
    bool isAlive() const { return m_hp > 0; }
    void takeDamage(int dmg) {
        m_hp -= dmg;
        if (m_hp < 0) m_hp = 0;
    }
    friend std::ostream& operator<<(std::ostream &os, const Enemy &e) {
        os << "[E:" << e.m_name << " HP:" << e.m_hp << " ATK:" << e.m_atk << " POS:" << e.m_pos << "]";
        return os;
    }
private:
    std::string m_name;
    int m_hp;
    int m_atk;
    int m_pos;
};

class CommandSequence {
public:
    CommandSequence() = default;
    void push(const std::string &cmd) {
        m_seq.push_back(cmd);
        if (m_seq.size() > m_maxHistory) m_seq.erase(m_seq.begin());
    }
    bool matchesMove() const { return endsWithPattern({ "pa", "pa", "pa", "po" }); }
    bool matchesAttack() const { return endsWithPattern({ "po", "po", "pa", "po" }); }
    void clear() { m_seq.clear(); }
    const std::vector<std::string>& getCommands() const { return m_seq; }
    friend std::ostream& operator<<(std::ostream &os, const CommandSequence &cs) {
        os << "Comenzi:";
        for (const auto &c : cs.m_seq) os << " " << c;
        return os;
    }
private:
    std::vector<std::string> m_seq;
    static constexpr std::size_t m_maxHistory = 8;
    bool endsWithPattern(const std::vector<std::string> &pattern) const {
        if (m_seq.size() < pattern.size()) return false;
        size_t n = m_seq.size(), m = pattern.size();
        for (size_t i = 0; i < m; ++i)
            if (m_seq[n - m + i] != pattern[i]) return false;
        return true;
    }
};

class Army {
public:
    Army(const std::vector<Patapon> &soldiers, int position = 0)
        : m_position(position) {
        m_soldiers.reserve(soldiers.size());
        for (const auto &s : soldiers) m_soldiers.push_back(new Patapon(s));
    }
    Army(const Army &other)
        : m_position(other.m_position) {
        m_soldiers.reserve(other.m_soldiers.size());
        for (const auto p : other.m_soldiers) m_soldiers.push_back(new Patapon(*p));
    }
    Army& operator=(const Army &other) {
        if (this != &other) {
            for (auto p : m_soldiers) delete p;
            m_soldiers.clear();
            m_position = other.m_position;
            m_soldiers.reserve(other.m_soldiers.size());
            for (const auto p : other.m_soldiers) m_soldiers.push_back(new Patapon(*p));
        }
        return *this;
    }
    ~Army() {
        for (auto p : m_soldiers) delete p;
        m_soldiers.clear();
    }
    void moveForward(int steps = 1) {
        if (steps <= 0) return;
        if (!hasLivingSoldiers()) return;
        m_position += steps;
    }
    void attackEnemies(std::vector<Enemy> &enemies) const {
        if (!hasLivingSoldiers()) return;
        std::ranges::sort(enemies, [](const Enemy &a, const Enemy &b){ return a.getPos() < b.getPos(); });
        for (auto &e : enemies) {
            if (!e.isAlive()) continue;
            int dist = e.getPos() - m_position;
            if (dist < 0) continue;
            int dmg = 0;
            for (const auto &p : m_soldiers) {
                if (!p->isAlive()) continue;
                int r = pataponRange(p);
                if (dist <= r) dmg += p->dealDamage();
            }
            if (dmg > 0) {
                e.takeDamage(dmg);
                if (e.isAlive()) {
                    int retaliate = std::max(1, e.getATK() - averageDefense());
                    for (auto &p : m_soldiers) {
                        if (p->isAlive()) {
                            p->takeDamage(retaliate);
                            break;
                        }
                    }
                }
                break;
            }
        }
    }
    void receiveEnemyAttack(int dmg) {
        for (auto &p : m_soldiers) {
            if (p->isAlive()) {
                p->takeDamage(dmg);
                break;
            }
        }
    }
    bool hasLivingSoldiers() const {
        for (const auto p : m_soldiers) if (p->isAlive()) return true;
        return false;
    }
    int getPosition() const { return m_position; }
    std::size_t size() const { return m_soldiers.size(); }
    friend std::ostream& operator<<(std::ostream &os, const Army &a) {
        os << "Pozitia armatei: " << a.m_position << "\n {";
        bool first = true;
        for (const auto p : a.m_soldiers) {
            if (!first) os << " | ";
            os << *p;
            first = false;
        }
        os << "}";
        return os;
    }
private:
    std::vector<Patapon*> m_soldiers;
    int m_position;
    static int pataponRange(const Patapon* s) {
        switch (s->getType()) {
            case Patapon::Type::SPEAR: return 2;
            case Patapon::Type::SHIELD: return 1;
            case Patapon::Type::BOW: return 3;
        }
        return 1;
    }
    int totalDamage() const {
        int sum = 0;
        for (const auto p : m_soldiers)
            if (p->isAlive()) sum += p->dealDamage();
        return sum;
    }
    int averageDefense() const {
        int sum = 0, count = 0;
        for (const auto p : m_soldiers) {
            if (p->isAlive()) { sum += p->getDEF(); ++count; }
        }
        return count ? (sum / count) : 0;
    }
};

class Game {
public:
    Game(const Army &army, const std::vector<Enemy> &enemies)
        : m_army(army), m_enemies(enemies), m_won(false), m_lost(false), m_turns(0) {
        // Goal is always at position 14 (last position on 15-unit map, 0-indexed)
        m_goal = GameConstants::MAP_SIZE - 1;
    }
    void processInput(const std::string &input) {
        if (m_won || m_lost) return;
        if (input != "pa" && input != "po") return;
        m_commands.push(input);
        update();
    }
    void update() {
        if (m_won || m_lost) return;
        if (m_commands.matchesMove()) {
            handleMove();
            m_commands.clear();
        } else if (m_commands.matchesAttack()) {
            handleAttack();
            m_commands.clear();
        }
        cleanupDeadEnemies();
        m_turns++;
        if (m_turns % 2 == 0) {
            enemiesAttack();
        } else {
            enemiesAdvance();
        }
        cleanupDeadEnemies();
        if (!m_army.hasLivingSoldiers()) {
            m_lost = true;
        } else if (m_army.getPosition() >= m_goal) {
            m_won = true;
        }
    }
    void render(std::ostream &os) const {
        os << "Inamici:\n";
        for (const auto &e : m_enemies) os << "  " << e << "\n";
        os << "\nArmata:\n  " << m_army << "\n";
        os << "\nComenzi:";
        for (const auto &c : m_commands.getCommands()) os << " " << c;
        os << "\n";
        os << "=== Camp ===\n";

        for (int pos = 0; pos < GameConstants::MAP_SIZE; ++pos) {
            if (pos == m_army.getPosition()) {
                os << "A";
                continue;
            }
            if (pos == m_goal) {
                os << "G";
                continue;
            }
            int count = 0;
            char c = 'E';
            for (const auto &e : m_enemies) {
                if (!e.isAlive()) continue;
                if (e.getPos() == pos) {
                    ++count;
                    if (count == 1 && !e.getName().empty()) c = std::toupper(static_cast<unsigned char>(e.getName()[0]));
                }
            }
            if (count == 0) os << ".";
            else if (count == 1) os << c;
            else if (count < 10) os << static_cast<char>('0' + count);
            else os << "M";
        }
        os << "\n";
        os << "---------------------------\n";
    }
    bool hasWon() const { return m_won; }
    bool hasLost() const { return m_lost; }
    friend std::ostream& operator<<(std::ostream &os, const Game &g) {
        g.render(os);
        return os;
    }
private:
    Army m_army;
    std::vector<Enemy> m_enemies;
    CommandSequence m_commands;
    bool m_won;
    bool m_lost;
    int m_turns;
    int m_goal;

    void handleMove() {
        bool allEnemiesDead = true;
        for (const auto &e : m_enemies) {
            if (e.isAlive()) {
                allEnemiesDead = false;
                break;
            }
        }

        int moveDistance = allEnemiesDead ? 3 : 1;
        int target = m_army.getPosition() + moveDistance;

        if (target >= GameConstants::MAP_SIZE) {
            target = GameConstants::MAP_SIZE - 1;
            moveDistance = target - m_army.getPosition();
            if (moveDistance <= 0) return;
        }

        for (int pos = m_army.getPosition() + 1; pos <= target; ++pos) {
            for (const auto &e : m_enemies) {
                if (!e.isAlive()) continue;
                if (e.getPos() == pos) return;
            }
        }

        m_army.moveForward(moveDistance);
    }
    void handleAttack() { m_army.attackEnemies(m_enemies); }
    void cleanupDeadEnemies() {
        m_enemies.erase(std::remove_if(m_enemies.begin(), m_enemies.end(),
            [](const Enemy &e){ return !e.isAlive(); }), m_enemies.end());
    }
    void enemiesAttack() {
        const int enemyAttackRange = 1;
        for (const auto &e : m_enemies) {
            if (!e.isAlive()) continue;
            int dist = std::abs(e.getPos() - m_army.getPosition());
            if (dist <= enemyAttackRange) {
                int dmg = std::max(1, e.getATK());
                m_army.receiveEnemyAttack(dmg);
            }
        }
    }
    void enemiesAdvance() {
        for (auto &e : m_enemies) {
            if (!e.isAlive()) continue;
            int armyPos = m_army.getPosition();
            if (e.getPos() > armyPos) {
                int desired = e.getPos() - 1;
                if (desired <= armyPos) continue;
                if (desired >= 0 && desired < GameConstants::MAP_SIZE) {
                    e.setPos(desired);
                }
            } else if (e.getPos() < armyPos) {
                int desired = e.getPos() + 1;
                if (desired >= armyPos) continue;
                if (desired >= 0 && desired < GameConstants::MAP_SIZE) {
                    e.setPos(desired);
                }
            }
        }
    }
};

int main() {
    using Type = Patapon::Type;
    Patapon p1(Type::SPEAR,  "Sulita",  20, 6, 1);
    Patapon p2(Type::SHIELD, "Scut",  25, 4, 3);
    Patapon p3(Type::BOW,    "Arc", 15, 5, 0);
    std::vector<Patapon> soldiers = { p1, p2, p3 };
    Army army(soldiers, 0);

    // Enemies positioned within the 15-unit map (positions 0-14)
    Enemy e1("Porc", 12, 4, 7);
    Enemy e2("Katapon", 18, 5, 11);
    std::vector<Enemy> enemies = { e1, e2 };

    Game game(army, enemies);
    std::cout << game;
    std::cout << "Tastati 'pa' sau 'po'. 'q' pentru iesire.\n";
    std::cout << "Exemplu: pa pa pa po  (miscare)\n";
    std::cout << "         po po pa po  (atac)\n\n";
    std::string token;
    while (std::cin >> token) {
        if (token == "q") break;
        game.processInput(token);
        std::cout << game;
        if (game.hasWon()) {
            std::cout << "Ai castigat! Ai ajuns la obiectiv.\n";
            break;
        }
        if (game.hasLost()) {
            std::cout << "Ai pierdut. Toti Pataponii au murit.\n";
            break;
        }
    }
    std::cout << "Joc incheiat.\n";
    return 0;
}