#pragma once
#include "Enemy.h"
#include <memory>

class Boss : public Enemy {
public:
    Boss(std::string name, int hp, int atk, int pos, int bonusDamage);
    Boss(const Boss& other);
    Boss& operator=(Boss other);
    ~Boss() override = default;

    friend void swap(Boss& first, Boss& second) noexcept;

    [[nodiscard]] std::unique_ptr<Unit> clone() const override;
    [[nodiscard]] int dealDamage() const override;
    [[nodiscard]] std::string getTypeLabel() const override;

    [[nodiscard]] int getBonusDamage() const;

protected:
    void renderDetails(std::ostream& os) const override;

private:
    int m_bonusDamage;
};
