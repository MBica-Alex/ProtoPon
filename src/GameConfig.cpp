#include "GameConfig.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

Patapon::Type GameConfig::parseType(const std::string& typeStr) {
    std::string upper = typeStr;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
    
    if (upper == "SPEAR") return Patapon::Type::SPEAR;
    if (upper == "SHIELD") return Patapon::Type::SHIELD;
    if (upper == "BOW") return Patapon::Type::BOW;
    
    throw InvalidInputException("Unknown Patapon type: " + typeStr);
}

std::vector<Patapon> GameConfig::loadSoldiers(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw ResourceLoadException("Failed to open config file: " + filename);
    }

    std::vector<Patapon> soldiers;
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;

        if (keyword == "SOLDIER") {
            std::string typeStr, name;
            int hp, atk, def;
            
            if (!(iss >> typeStr >> name >> hp >> atk >> def)) {
                throw InvalidInputException("Invalid SOLDIER format in config");
            }

            Patapon::Type type = parseType(typeStr);
            soldiers.emplace_back(type, name, hp, atk, def);
        }
    }

    if (soldiers.empty()) {
        throw InvalidStateException("No soldiers found in config file");
    }

    return soldiers;
}
