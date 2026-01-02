#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Patapon.h"
#include "Enemy.h"
#include "Boss.h"
#include "GameException.h"

class GameConfig {
public:
    static std::vector<Patapon> loadSoldiers(const std::string& filename);

private:
    static Patapon::Type parseType(const std::string& typeStr);
};
