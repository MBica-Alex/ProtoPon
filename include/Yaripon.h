#pragma once
#include "Patapon.h"

class Yaripon : public Patapon {
public:
    Yaripon(std::string name, int max_hp, int atk, int def);
    
    [[nodiscard]] std::unique_ptr<Unit> clone() const override;
    [[nodiscard]] int getRange() const override;
    
protected:
    void doPrintInfo() const override;
};
