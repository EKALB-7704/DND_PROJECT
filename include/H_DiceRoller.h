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

struct DiceExpression {
    // A written damage roll such as "2d6+1": count dice of `sides`, plus bonus.
    // Flat damage like a blowgun's "1" has count 0 and only a bonus.
    int count = 0;
    int sides = 0;
    int bonus = 0;
};

struct DamageRollResult {
    std::vector<int> rolls;  // each die rolled, doubled in number on a critical
    int modifier;            // ability modifier plus the expression's own bonus
    int total;               // never below 0
};

// Parses "NdM", "dM", either with an optional "+K"/"-K", or a flat "K".
// Case and spaces are ignored. Dice counts and sides must be 1-100, and every
// number at most three digits. Returns false (leaving `out` untouched) for
// anything else, such as "1d8/1d10".
bool parseDiceExpression(const std::string& text, DiceExpression& out);

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

    // Rolls a damage expression and adds `modifier`. A critical hit rolls
    // the dice twice over; the modifier and bonus are added once.
    DamageRollResult rollDamage(const DiceExpression& expr, int modifier, bool critical);

    // Handles user prompts for choosing the die type and displaying results.
    void promptAndRoll(ConsoleIO& io);
};

#endif
