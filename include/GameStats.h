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



    [[nodiscard]] int getDamageDealt() const;
    [[nodiscard]] int getDamageTaken() const;
    [[nodiscard]] int getCommandsCount() const;
    [[nodiscard]] int getStepsTaken() const;
    [[nodiscard]] int getTurns() const;

private:
    int damageDealt;
    int damageTaken;
    int commandsCount;
    int stepsTaken;
    int turns;
};
