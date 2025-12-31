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

    [[nodiscard]] const std::string& getName() const;
    [[nodiscard]] int getHP() const;
    [[nodiscard]] int getMaxHP() const;
    [[nodiscard]] int getATK() const;
    [[nodiscard]] bool isAlive() const;

protected:
    virtual void renderDetails(std::ostream& os) const = 0;

    std::string m_name;
    int m_hp;
    int m_max_hp;
    int m_atk;
};
