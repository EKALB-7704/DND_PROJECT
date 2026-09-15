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

// ---------------------------------------------------------------------------
// Damage expressions and damage rolls
// ---------------------------------------------------------------------------

namespace {
DiceExpression parsed(const std::string& text) {
    DiceExpression e;
    EXPECT_TRUE(parseDiceExpression(text, e)) << text;
    return e;
}
}

TEST(DiceExpressionTest, ParsesTheFormsWeaponsAreWrittenIn) {
    DiceExpression e = parsed("1d6");
    EXPECT_EQ(e.count, 1); EXPECT_EQ(e.sides, 6); EXPECT_EQ(e.bonus, 0);

    e = parsed("d8");                         // count defaults to one die
    EXPECT_EQ(e.count, 1); EXPECT_EQ(e.sides, 8);

    e = parsed("2d6+1");
    EXPECT_EQ(e.count, 2); EXPECT_EQ(e.sides, 6); EXPECT_EQ(e.bonus, 1);

    e = parsed(" 1D10 - 2 ");                 // spacing and case are ignored
    EXPECT_EQ(e.count, 1); EXPECT_EQ(e.sides, 10); EXPECT_EQ(e.bonus, -2);

    e = parsed("1");                          // flat damage, e.g. a blowgun
    EXPECT_EQ(e.count, 0); EXPECT_EQ(e.bonus, 1);
}

TEST(DiceExpressionTest, RejectsAnythingItCannotRollExactly) {
    for (const char* bad : {"", "d", "1d", "0d6", "1d0", "1d101", "101d6",
                            "1d8/1d10", "2d6+", "1d6+1+1", "abc", "9999d6", "1d6x"}) {
        DiceExpression e;
        e.count = 42;
        EXPECT_FALSE(parseDiceExpression(bad, e)) << bad;
        EXPECT_EQ(e.count, 42) << "output must be untouched on failure: " << bad;
    }
}

TEST(DiceRollerTest, RollDamageAddsModifierAndExpressionBonus) {
    DiceRoller roller;
    // A d1 always rolls 1, so the total is exact: 2 dice + 3 bonus + 2 mod.
    const DamageRollResult r = roller.rollDamage(parsed("2d1+3"), 2, false);
    EXPECT_EQ(r.rolls.size(), 2u);
    EXPECT_EQ(r.modifier, 5);
    EXPECT_EQ(r.total, 7);
}

TEST(DiceRollerTest, RollDamageCriticalDoublesDiceButNotModifier) {
    DiceRoller roller;
    const DamageRollResult r = roller.rollDamage(parsed("2d1+3"), 2, true);
    EXPECT_EQ(r.rolls.size(), 4u);
    EXPECT_EQ(r.modifier, 5);
    EXPECT_EQ(r.total, 9);

    const DamageRollResult real = roller.rollDamage(parsed("1d8"), 0, true);
    ASSERT_EQ(real.rolls.size(), 2u);
    for (int roll : real.rolls) { EXPECT_GE(roll, 1); EXPECT_LE(roll, 8); }
    EXPECT_EQ(real.total, real.rolls[0] + real.rolls[1]);
}

TEST(DiceRollerTest, RollDamageNeverGoesBelowZero) {
    DiceRoller roller;
    const DamageRollResult r = roller.rollDamage(parsed("1d1"), -5, false);
    EXPECT_EQ(r.modifier, -5);
    EXPECT_EQ(r.total, 0);
}

TEST(DiceRollerTest, RollDamageFlatHasNoDiceEvenOnACritical) {
    DiceRoller roller;
    const DamageRollResult r = roller.rollDamage(parsed("1"), 0, true);
    EXPECT_TRUE(r.rolls.empty());
    EXPECT_EQ(r.total, 1);
}
