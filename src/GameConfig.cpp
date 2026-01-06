#include "GameConfig.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>



std::vector<std::unique_ptr<Patapon>> GameConfig::loadSoldiers(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw ResourceLoadException("Failed to open config file: " + filename);
    }

    std::vector<std::unique_ptr<Patapon>> soldiers;
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
            
            std::string upper = typeStr;
            std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

            if (upper == "SPEAR") {
                 soldiers.push_back(std::make_unique<Yaripon>(name, hp, atk, def));
            } else if (upper == "SHIELD") {
                 soldiers.push_back(std::make_unique<Tatepon>(name, hp, atk, def));
            } else if (upper == "BOW") {
                 soldiers.push_back(std::make_unique<Yumipon>(name, hp, atk, def));
            } else {
                 throw InvalidInputException("Unknown Patapon type: " + typeStr);
            }
        }
    }

    if (soldiers.empty()) {
        throw InvalidStateException("No soldiers found in config file");
    }

    return soldiers;
}
