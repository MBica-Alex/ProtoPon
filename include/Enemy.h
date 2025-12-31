#pragma once
#include "Unit.h"
#include "GameException.h"
#include <memory>

class Enemy : public Unit {
public:
    Enemy(std::string name, int hp, int atk, int pos);
    Enemy(const Enemy& other);
    Enemy& operator=(Enemy other);
    ~Enemy() override = default;

    friend void swap(Enemy& first, Enemy& second) noexcept;

    [[nodiscard]] std::unique_ptr<Unit> clone() const override;
    [[nodiscard]] int dealDamage() const override;
    [[nodiscard]] std::string getTypeLabel() const override;

    [[nodiscard]] int getPos() const;
    void setPos(int p);

protected:
    void renderDetails(std::ostream& os) const override;
    int m_pos;
};
