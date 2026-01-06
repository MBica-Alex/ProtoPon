#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Patapon.h"
#include "Yaripon.h"
#include "Tatepon.h"
#include "Yumipon.h"
#include "GameException.h"

class GameConfig {
public:
    static std::vector<std::unique_ptr<Patapon>> loadSoldiers(const std::string& filename);
};
