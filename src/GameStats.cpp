#include "GameStats.h"

GameStats::GameStats() : damageDealt(0), damageTaken(0), commandsCount(0), stepsTaken(0), turns(0) {}

GameStats::GameStats(int dealt, int taken, int commands, int steps, int turnsCount)
    : damageDealt(dealt), damageTaken(taken), commandsCount(commands),
      stepsTaken(steps), turns(turnsCount) {}

void GameStats::addDamageDealt(int amount) { damageDealt += amount; }
void GameStats::addDamageTaken(int amount) { damageTaken += amount; }
void GameStats::addCommand() { commandsCount++; }
void GameStats::addSteps(int steps) { stepsTaken += steps; }
void GameStats::addTurn() { turns++; }
