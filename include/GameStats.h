#pragma once
#include <iostream>

class GameStats {
public:
    GameStats();
    GameStats(int dealt, int taken, int commands, int steps, int turnsCount);

    void addDamageDealt(int amount);
    void addDamageTaken(int amount);
    void addCommand();
    void addSteps(int steps);
    void addTurn();

    [[nodiscard]] int getDamageDealt() const { return damageDealt; }
    [[nodiscard]] int getDamageTaken() const { return damageTaken; }
    [[nodiscard]] int getCommandsCount() const { return commandsCount; }
    [[nodiscard]] int getStepsTaken() const { return stepsTaken; }
    [[nodiscard]] int getTurns() const { return turns; }

private:
    int damageDealt;
    int damageTaken;
    int commandsCount;
    int stepsTaken;
    int turns;
};
