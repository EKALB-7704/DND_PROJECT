#ifndef DICE_ROLLER_H
#define DICE_ROLLER_H

#include <random>
#include <string>
#include <vector>

#include "H_ConsoleIO.h"

enum class D20Mode {
    Normal = 0,
    Advantage = 1,
    Disadvantage = 2
};

struct D20RollResult {
    // For advantage/disadvantage we keep both raw rolls and the chosen outcome.
    int firstRoll;
    int secondRoll;
    int chosenRoll;
};

struct CheckRollResult {
    // A d20 test: the die (with both rolls, for advantage/disadvantage), the
    // modifier added to it, and the total the table compares against a DC.
    D20RollResult d20;
    int modifier;
    int total;
};

class DiceRoller {
private:
    // Reused random-number engine for all dice rolls in this session.
    std::mt19937 rng;

public:
    DiceRoller();

    // Core rolling helpers used by the interactive menu and the test suite.
    int rollDie(int sides);
    std::vector<int> rollDice(int count, int sides);
    int totalRoll(const std::vector<int>& rolls) const;
    D20RollResult rollD20(D20Mode mode);

    // Skill checks, saving throws, ability checks and initiative are all a
    // d20 plus a modifier; this rolls one and adds the modifier.
    CheckRollResult rollCheck(D20Mode mode, int modifier);

    // Handles user prompts for choosing the die type and displaying results.
    void promptAndRoll(ConsoleIO& io);
};

#endif
