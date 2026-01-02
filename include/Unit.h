#pragma once
#include <string>
#include <memory>
#include <iostream>

class Unit {
public:
    Unit(std::string name, int hp, int atk);
    Unit(const Unit& other);
    Unit& operator=(const Unit& other);
    virtual ~Unit() = default;

    [[nodiscard]] virtual std::unique_ptr<Unit> clone() const = 0;
    [[nodiscard]] virtual int dealDamage() const = 0;
    [[nodiscard]] virtual std::string getTypeLabel() const = 0;
    virtual void takeDamage(int dmg);

    void print(std::ostream& os) const;
    friend std::ostream& operator<<(std::ostream& os, const Unit& unit);

    [[nodiscard]] const std::string& getName() const { return m_name; }
    [[nodiscard]] int getHP() const { return m_hp; }
    [[nodiscard]] int getMaxHP() const { return m_max_hp; }
    [[nodiscard]] bool isAlive() const { return m_hp > 0; }

protected:
    virtual void renderDetails(std::ostream& os) const = 0;

    std::string m_name;
    int m_hp;
    int m_max_hp;
    int m_atk;
};
