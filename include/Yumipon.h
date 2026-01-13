#pragma once
#include "Patapon.h"

class Yumipon : public Patapon {
public:
    Yumipon(std::string name, int max_hp, int atk, int def);
    
    [[nodiscard]] std::unique_ptr<Unit> clone() const override;
    [[nodiscard]] int getRange() const override;
    [[nodiscard]] int dealDamage() const override;
    
};
