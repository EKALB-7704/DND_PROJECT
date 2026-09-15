#include "H_DiceRoller.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <iostream>
#include <limits>
#include <numeric>
#include <thread>


// Dice in DND are a core part of the game and they are used for a wide variety of tasks, such as Skill checks, Damage, saving throws, Healing etc.
// DND has 6 unique die: d4. d6. d8, d10, d12 and d20 

// sometimes you can be asked to roll a d2, d3 or a d100, though these are normally substituted with existing die with addtional conditions.
// for a d2, generally, you roll a d6 with the odd values representing 1, and the evens, 2. 
// for a d3, generally, you roll a d4, with 1-3 representing their values as normal, and 4 meaning roll again.
// A d100 is typically rolled using 2 d10 die, one for the tens and one for the units. Some sets even come with special d10 die to represent this

// Sometimes when rolling a d20, you may be asked to roll at "advantage" or "disadvantage". This means that you must roll 2 d20
// and use the  higher (advantage) or lower (disadvantage) number. This mechanic in-game is used to reflect whether the conditions are ideal or sub-optimal
// when making a skill check

// when having to roll multiple dice for an action, such as for damage, you will see it represented something like 3d8. this just means 3 d8 die 
// must be rolled and their individual values totalled to use for whatever action you are trying to do 


namespace {
// Restrict the interactive roller to the standard dice used in this project.
// Single source of truth: the prompt validates against this same list.
const std::vector<int> kSupportedDice = {4, 6, 8, 10, 12, 20, 100};

// Reads one to three digits starting at `pos`, advancing past them. Capping
// the length keeps std::stoi clear of out_of_range.
bool readSmallNumber(const std::string& text, size_t& pos, int& out)
{
    const size_t start = pos;
    while (pos < text.size() && std::isdigit(static_cast<unsigned char>(text[pos])))
    {
        pos++;
    }
    const size_t length = pos - start;
    if (length == 0 || length > 3) return false;
    out = std::stoi(text.substr(start, length));
    return true;
}

// Small timed animation used only by the interactive menu flow.
void showRollAnimation()
{
    std::cout << "Rolling";
    for (int i = 0; i < 6; i++)
    {
        std::cout << "#" << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
    }
    std::cout << "\n";
}
}

// Set up one random engine for all rolls made during this run.
DiceRoller::DiceRoller()
    // Seed from the current time so each run produces different rolls.
    : rng(static_cast<std::mt19937::result_type>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count()))
{
}

// Roll a single die with the requested number of sides.
int DiceRoller::rollDie(int sides)
{
    // Produce one random result in the inclusive range [1, sides].
    std::uniform_int_distribution<int> distribution(1, sides);
    return distribution(rng);
}

// Roll multiple copies of the same die and store each result.
std::vector<int> DiceRoller::rollDice(int count, int sides)
{
    std::vector<int> rolls;
    // Roll the same die multiple times and keep each individual result.
    for (int i = 0; i < count; i++)
    {
        rolls.push_back(rollDie(sides));
    }
    return rolls;
}

// Add all rolled values together for a final total.
int DiceRoller::totalRoll(const std::vector<int>& rolls) const
{
    // Sum all individual dice into the final combined total.
    return std::accumulate(rolls.begin(), rolls.end(), 0);
}

// Handle normal, advantage, and disadvantage d20 rolls.
D20RollResult DiceRoller::rollD20(D20Mode mode)
{
    D20RollResult result{};
    result.firstRoll = rollDie(20);
    result.secondRoll = rollDie(20);

    // Advantage keeps the higher roll, disadvantage keeps the lower roll.
    if (mode == D20Mode::Advantage)
    {
        result.chosenRoll = std::max(result.firstRoll, result.secondRoll);
    }
    else if (mode == D20Mode::Disadvantage)
    {
        result.chosenRoll = std::min(result.firstRoll, result.secondRoll);
    }
    else
    {
        result.chosenRoll = result.firstRoll;
    }

    return result;
}

// Roll a d20 test and add its modifier.
CheckRollResult DiceRoller::rollCheck(D20Mode mode, int modifier)
{
    CheckRollResult result{};
    result.d20 = rollD20(mode);
    result.modifier = modifier;
    result.total = result.d20.chosenRoll + modifier;
    return result;
}

// Parse a written damage roll like "2d6+1", "d8" or a flat "1".
bool parseDiceExpression(const std::string& text, DiceExpression& out)
{
    // Weapon damage is free text, so ignore spacing and the case of the 'd'.
    std::string s;
    for (char ch : text)
    {
        if (!std::isspace(static_cast<unsigned char>(ch)))
        {
            s += static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        }
    }

    DiceExpression expr;
    size_t pos = 0;
    const size_t d = s.find('d');

    if (d == std::string::npos)
    {
        // No dice at all: flat damage.
        if (!readSmallNumber(s, pos, expr.bonus) || pos != s.size()) return false;
        out = expr;
        return true;
    }

    // "d8" means one die; otherwise the count must run right up to the 'd'.
    if (d == 0)
    {
        expr.count = 1;
    }
    else if (!readSmallNumber(s, pos, expr.count) || pos != d)
    {
        return false;
    }

    pos = d + 1;
    if (!readSmallNumber(s, pos, expr.sides)) return false;

    if (pos < s.size())
    {
        const char sign = s[pos++];
        int bonus = 0;
        if ((sign != '+' && sign != '-') || !readSmallNumber(s, pos, bonus)) return false;
        expr.bonus = (sign == '-') ? -bonus : bonus;
    }

    if (pos != s.size()) return false;
    if (expr.count < 1 || expr.count > 100 || expr.sides < 1 || expr.sides > 100) return false;

    out = expr;
    return true;
}

// Roll damage, doubling the dice (not the modifier) on a critical hit.
DamageRollResult DiceRoller::rollDamage(const DiceExpression& expr, int modifier, bool critical)
{
    DamageRollResult result{};
    if (expr.count > 0)
    {
        result.rolls = rollDice(critical ? expr.count * 2 : expr.count, expr.sides);
    }
    result.modifier = modifier + expr.bonus;
    // A penalty can outweigh a low roll, but damage never heals the target.
    result.total = std::max(0, totalRoll(result.rolls) + result.modifier);
    return result;
}

// Main interactive menu flow for the standalone dice roller.
void DiceRoller::promptAndRoll(ConsoleIO& io)
{
    std::ostream& out = io.os();

    out << "\n=== Dice Roller ===\n";
    // Each of these prompts used to abort the whole roll on a bad entry;
    // they now re-prompt until the value is usable.
    const int sides = io.readIntFrom("Choose a die (4, 6, 8, 10, 12, 20, 100): ",
                                     kSupportedDice);

    if (sides == 20)
    {
        // d20 rolls get a separate prompt for advantage/disadvantage handling.
        out << "1. Normal\n";
        out << "2. Advantage\n";
        out << "3. Disadvantage\n";
        const int modeChoice = io.readInt("Choice: ", 1, 3);

        D20Mode mode = D20Mode::Normal;
        if (modeChoice == 2)
        {
            mode = D20Mode::Advantage;
        }
        else if (modeChoice == 3)
        {
            mode = D20Mode::Disadvantage;
        }

        showRollAnimation();
        D20RollResult result = rollD20(mode);

        if (mode == D20Mode::Normal)
        {
            out << "Result: " << result.chosenRoll << "\n";
        }
        else
        {
            // Show both d20 rolls so the player can see what advantage/disadvantage did.
            out << "Rolls: " << result.firstRoll << ", " << result.secondRoll << "\n";
            out << "Chosen result: " << result.chosenRoll << "\n";
        }
        return;
    }

    if (sides == 100)
    {
        // d100 is always rolled once, so no quantity prompt is needed.
        showRollAnimation();
        out << "Result: " << rollDie(100) << "\n";
        return;
    }

    // All other supported dice ask how many copies of that die to roll.
    const int count = io.readInt(
        "How many d" + std::to_string(sides) + " would you like to roll? ", 1, 100);

    showRollAnimation();
    std::vector<int> rolls = rollDice(count, sides);
    out << "Rolls: ";
    for (size_t i = 0; i < rolls.size(); i++)
    {
        out << rolls[i];
        if (i + 1 < rolls.size())
        {
            out << ", ";
        }
    }
    out << "\nTotal: " << totalRoll(rolls) << "\n";
}
