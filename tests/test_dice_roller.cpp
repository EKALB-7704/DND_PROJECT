#include <gtest/gtest.h>
#include "H_DiceRoller.h"

// Basic range check: a d6 should never produce values outside 1-6.
TEST(DiceRollerTest, RollDieStaysWithinExpectedRange) {
    DiceRoller roller;
    for (int i = 0; i < 100; i++)
    {
        const int roll = roller.rollDie(6);
        EXPECT_GE(roll, 1);
        EXPECT_LE(roll, 6);
    }
}

// Rolling multiple dice should return one entry per die requested.
TEST(DiceRollerTest, RollDiceReturnsRequestedCount) {
    DiceRoller roller;
    const auto rolls = roller.rollDice(4, 8);
    ASSERT_EQ(rolls.size(), 4u);
    for (int roll : rolls)
    {
        EXPECT_GE(roll, 1);
        EXPECT_LE(roll, 8);
    }
}

// Combined totals are used by the interactive display for multi-die rolls.
TEST(DiceRollerTest, TotalRollAddsAllValues) {
    DiceRoller roller;
    EXPECT_EQ(roller.totalRoll({3, 4, 5}), 12);
}

// A normal d20 roll reports the first roll as the final result.
TEST(DiceRollerTest, D20NormalUsesFirstRollAsChosenRoll) {
    DiceRoller roller;
    const D20RollResult result = roller.rollD20(D20Mode::Normal);
    EXPECT_EQ(result.chosenRoll, result.firstRoll);
    EXPECT_GE(result.firstRoll, 1);
    EXPECT_LE(result.firstRoll, 20);
}

// Advantage should always choose the higher of the two raw rolls.
TEST(DiceRollerTest, D20AdvantageUsesHigherRoll) {
    DiceRoller roller;
    const D20RollResult result = roller.rollD20(D20Mode::Advantage);
    EXPECT_EQ(result.chosenRoll, std::max(result.firstRoll, result.secondRoll));
}

// Disadvantage should always choose the lower of the two raw rolls.
TEST(DiceRollerTest, D20DisadvantageUsesLowerRoll) {
    DiceRoller roller;
    const D20RollResult result = roller.rollD20(D20Mode::Disadvantage);
    EXPECT_EQ(result.chosenRoll, std::min(result.firstRoll, result.secondRoll));
}

// A check's total is always the chosen d20 plus the modifier, in every mode.
TEST(DiceRollerTest, RollCheckAddsModifierToChosenRoll) {
    DiceRoller roller;
    for (D20Mode mode : {D20Mode::Normal, D20Mode::Advantage, D20Mode::Disadvantage}) {
        const CheckRollResult result = roller.rollCheck(mode, 4);
        EXPECT_EQ(result.modifier, 4);
        EXPECT_EQ(result.total, result.d20.chosenRoll + 4);
        EXPECT_GE(result.d20.chosenRoll, 1);
        EXPECT_LE(result.d20.chosenRoll, 20);
    }
}

// Negative modifiers subtract, and advantage still keeps the higher die.
TEST(DiceRollerTest, RollCheckHandlesNegativeModifierWithAdvantage) {
    DiceRoller roller;
    const CheckRollResult result = roller.rollCheck(D20Mode::Advantage, -2);
    EXPECT_EQ(result.d20.chosenRoll, std::max(result.d20.firstRoll, result.d20.secondRoll));
    EXPECT_EQ(result.total, result.d20.chosenRoll - 2);
}
