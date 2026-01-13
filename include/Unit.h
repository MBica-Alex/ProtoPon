#pragma once
#include <string>
#include <memory>


class Unit {
public:
    Unit(std::string name, int hp, int atk);
    Unit(const Unit& other) = default;
    Unit& operator=(const Unit& other) = default;
    virtual ~Unit() = default;

    [[nodiscard]] virtual std::unique_ptr<Unit> clone() const = 0;
    [[nodiscard]] virtual int dealDamage() const = 0;

    virtual void takeDamage(int dmg);



    [[nodiscard]] const std::string& getName() const { return m_name; }
    [[nodiscard]] int getHP() const { return m_hp; }
    [[nodiscard]] int getMaxHP() const { return m_max_hp; }
    [[nodiscard]] int getATK() const { return m_atk; }
    [[nodiscard]] bool isAlive() const { return m_hp > 0; }

protected:
    std::string m_name;
    int m_hp;
    int m_max_hp;
    int m_atk;
};
