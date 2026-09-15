#include <gtest/gtest.h>
#include "H_Character.h"

// Helper: builds a minimal valid Character for use across tests
static Character makeCharacter(
    const std::string& name  = "Aragorn",
    const std::string& race  = "Human",
    const std::string& cls   = "Ranger",
    int level                = 5,
    int str = 16, int dex = 14, int con = 14,
    int intel = 10, int wis = 12, int cha = 10)
{
    return Character(
        name, race, cls,
        /*background*/ "Outlander", /*alignment*/ "Neutral Good",
        level, /*age*/ 25, /*weight*/ 180,
        /*c_hp*/ 40, /*m_hp*/ 40, /*t_hp*/ 0,
        /*hitDice*/ "5d10",
        str, dex, con, intel, wis, cha,
        /*initiative*/ 2, /*proficiency*/ 3
    );
}

// ── Getters reflect constructor values ───────────────────────────────────────

TEST(CharacterTest, ConstructorSetsName) {
    Character c = makeCharacter("Legolas");
    EXPECT_EQ(c.getName(), "Legolas");
}

TEST(CharacterTest, ConstructorSetsRaceAndClass) {
    Character c = makeCharacter("Gimli", "Dwarf", "Fighter");
    EXPECT_EQ(c.getRace(),  "Dwarf");
    EXPECT_EQ(c.getClass(), "Fighter");
}

TEST(CharacterTest, ConstructorSetsLevel) {
    Character c = makeCharacter("Frodo", "Halfling", "Rogue", /*level*/ 3);
    EXPECT_EQ(c.getLevel(), 3);
}

TEST(CharacterTest, ConstructorSetsAbilityScores) {
    Character c = makeCharacter("Test", "Human", "Fighter", 1,
                                /*str*/ 18, /*dex*/ 12, /*con*/ 16,
                                /*int*/ 8,  /*wis*/ 10, /*cha*/ 14);
    EXPECT_EQ(c.getStrength(),     18);
    EXPECT_EQ(c.getDexterity(),    12);
    EXPECT_EQ(c.getConstitution(), 16);
    EXPECT_EQ(c.getIntelligence(),  8);
    EXPECT_EQ(c.getWisdom(),       10);
    EXPECT_EQ(c.getCharisma(),     14);
}

TEST(CharacterTest, ConstructorSetsHP) {
    Character c = makeCharacter();
    EXPECT_EQ(c.getCurrentHP(), 40);
    EXPECT_EQ(c.getMaxHP(),     40);
    EXPECT_EQ(c.getTempHP(),     0);
}

// ── Setters mutate correctly ──────────────────────────────────────────────────

TEST(CharacterTest, SetNameUpdatesName) {
    Character c = makeCharacter("OldName");
    c.setName("NewName");
    EXPECT_EQ(c.getName(), "NewName");
}

TEST(CharacterTest, SetLevelUpdatesLevel) {
    Character c = makeCharacter();
    c.setLevel(10);
    EXPECT_EQ(c.getLevel(), 10);
}

TEST(CharacterTest, SetCurrentHPUpdatesHP) {
    Character c = makeCharacter();
    c.setCurrentHP(15);
    EXPECT_EQ(c.getCurrentHP(), 15);
}

TEST(CharacterTest, SetMaxHPUpdatesMaxHP) {
    Character c = makeCharacter();
    c.setMaxHP(55);
    EXPECT_EQ(c.getMaxHP(), 55);
}

TEST(CharacterTest, DeathSavesClampToValidRange) {
    Character c = makeCharacter();
    c.setDeathSaveSuccesses(7);
    c.setDeathSaveFailures(9);

    EXPECT_EQ(c.getDeathSaveSuccesses(), 3);
    EXPECT_EQ(c.getDeathSaveFailures(), 3);
}

TEST(CharacterTest, HealingAboveZeroResetsDeathSaves) {
    Character c = makeCharacter();
    c.setDeathSaveSuccesses(2);
    c.setDeathSaveFailures(1);

    c.setCurrentHP(5);

    EXPECT_EQ(c.getDeathSaveSuccesses(), 0);
    EXPECT_EQ(c.getDeathSaveFailures(), 0);
}

TEST(CharacterTest, DeathSavePassAddsOneSuccess) {
    Character c = makeCharacter();
    c.setCurrentHP(0);

    const DeathSaveOutcome outcome = c.applyDeathSaveRoll(14);

    EXPECT_EQ(outcome, DeathSaveOutcome::None);
    EXPECT_EQ(c.getDeathSaveSuccesses(), 1);
    EXPECT_EQ(c.getDeathSaveFailures(), 0);
}

TEST(CharacterTest, DeathSaveFailureAddsOneFailure) {
    Character c = makeCharacter();
    c.setCurrentHP(0);

    const DeathSaveOutcome outcome = c.applyDeathSaveRoll(7);

    EXPECT_EQ(outcome, DeathSaveOutcome::None);
    EXPECT_EQ(c.getDeathSaveSuccesses(), 0);
    EXPECT_EQ(c.getDeathSaveFailures(), 1);
}

TEST(CharacterTest, DeathSaveNaturalOneCountsAsTwoFailures) {
    Character c = makeCharacter();
    c.setCurrentHP(0);

    const DeathSaveOutcome outcome = c.applyDeathSaveRoll(1);

    EXPECT_EQ(outcome, DeathSaveOutcome::None);
    EXPECT_EQ(c.getDeathSaveSuccesses(), 0);
    EXPECT_EQ(c.getDeathSaveFailures(), 2);
}

TEST(CharacterTest, DeathSaveNaturalTwentyRestoresOneHpAndResetsTracker) {
    Character c = makeCharacter();
    c.setCurrentHP(0);
    c.setDeathSaveSuccesses(2);
    c.setDeathSaveFailures(2);

    const DeathSaveOutcome outcome = c.applyDeathSaveRoll(20);

    EXPECT_EQ(outcome, DeathSaveOutcome::Revived);
    EXPECT_EQ(c.getCurrentHP(), 1);
    EXPECT_EQ(c.getDeathSaveSuccesses(), 0);
    EXPECT_EQ(c.getDeathSaveFailures(), 0);
}

TEST(CharacterTest, ThreeDeathSaveSuccessesCauseStableAndResetTracker) {
    Character c = makeCharacter();
    c.setCurrentHP(0);
    c.setDeathSaveSuccesses(2);

    const DeathSaveOutcome outcome = c.applyDeathSaveRoll(12);

    EXPECT_EQ(outcome, DeathSaveOutcome::Stable);
    EXPECT_EQ(c.getDeathSaveSuccesses(), 0);
    EXPECT_EQ(c.getDeathSaveFailures(), 0);
}

TEST(CharacterTest, ThreeDeathSaveFailuresCauseDeathAndResetTracker) {
    Character c = makeCharacter();
    c.setCurrentHP(0);
    c.setDeathSaveFailures(2);

    const DeathSaveOutcome outcome = c.applyDeathSaveRoll(4);

    EXPECT_EQ(outcome, DeathSaveOutcome::Dead);
    EXPECT_EQ(c.getDeathSaveSuccesses(), 0);
    EXPECT_EQ(c.getDeathSaveFailures(), 0);
}

TEST(CharacterTest, ConditionsCanBeAddedAndRemoved) {
    Character c = makeCharacter();
    c.addCondition("Poisoned");
    c.addCondition("Blinded");

    ASSERT_EQ(c.getConditions().size(), 2u);
    EXPECT_EQ(c.getConditions()[0], "Poisoned");
    EXPECT_TRUE(c.removeCondition(1));
    ASSERT_EQ(c.getConditions().size(), 1u);
    EXPECT_EQ(c.getConditions()[0], "Blinded");
}

TEST(CharacterTest, PassivePerceptionUsesWisdomAndPerceptionProficiency) {
    Character c = makeCharacter("Scout", "Elf", "Ranger", 5,
                                /*str*/ 10, /*dex*/ 14, /*con*/ 12,
                                /*int*/ 10, /*wis*/ 16, /*cha*/ 10);

    EXPECT_EQ(c.getPassivePerception(), 13);

    c.getFeatures().setSkillRank("Perception", SkillRank::Proficient);
    EXPECT_EQ(c.getPassivePerception(), 16);

    c.getFeatures().setSkillRank("Perception", SkillRank::Expertise);
    EXPECT_EQ(c.getPassivePerception(), 19);
}

// Skill and save modifiers come from the character's own scores and proficiency.
TEST(CharacterTest, SkillModifierUsesLinkedAbilityAndRank) {
    Character c = makeCharacter();              // DEX 14 (+2), proficiency +3
    EXPECT_EQ(c.getSkillModifier("Stealth"), 2);

    c.getFeatures().setSkillRank("Stealth", SkillRank::Proficient);
    EXPECT_EQ(c.getSkillModifier("Stealth"), 5);

    c.getFeatures().setSkillRank("Stealth", SkillRank::Expertise);
    EXPECT_EQ(c.getSkillModifier("Stealth"), 8);
}

TEST(CharacterTest, SaveModifierAddsProficiencyOnlyWhenProficient) {
    Character c = makeCharacter();              // STR 16 (+3), proficiency +3
    EXPECT_EQ(c.getSaveModifier("STR"), 3);

    c.getFeatures().setSaveProficiency("STR", true);
    EXPECT_EQ(c.getSaveModifier("STR"), 6);
}

TEST(CharacterTest, UnknownSkillOrSaveHasZeroModifier) {
    Character c = makeCharacter();
    EXPECT_EQ(c.getSkillModifier("Juggling"), 0);
    EXPECT_EQ(c.getSaveModifier("LUCK"), 0);
}

// The hit die's sides drive the short-rest roll; anything unrollable is 0.
TEST(CharacterTest, HitDieSidesParsesDnAndRejectsUnrollableValues) {
    Character c = makeCharacter();
    c.setHitDice("d10");
    EXPECT_EQ(c.getHitDieSides(), 10);
    c.setHitDice("d6");
    EXPECT_EQ(c.getHitDieSides(), 6);

    c.setHitDice("d0");          // passes the dN format check, but has no sides
    EXPECT_EQ(c.getHitDieSides(), 0);
    c.setHitDice("5d10");        // count-prefixed form is not a stored hit die
    EXPECT_EQ(c.getHitDieSides(), 0);
    c.setHitDice("d99999999999"); // would overflow std::stoi
    EXPECT_EQ(c.getHitDieSides(), 0);
}

// ── Ability modifier formula: floor(score/2) - 5 ─────────────────────────────
// D&D standard modifiers: 10→+0, 12→+1, 8→-1, 20→+5, 1→-5

TEST(CharacterTest, AbilityModifierScore10IsZero) {
    Character c = makeCharacter();
    int score = 10;
    EXPECT_EQ(c.getAbilityModifier(score), 0);
}

TEST(CharacterTest, AbilityModifierScore12IsPlusOne) {
    Character c = makeCharacter();
    int score = 12;
    EXPECT_EQ(c.getAbilityModifier(score), 1);
}

TEST(CharacterTest, AbilityModifierScore8IsMinusOne) {
    Character c = makeCharacter();
    int score = 8;
    EXPECT_EQ(c.getAbilityModifier(score), -1);
}

TEST(CharacterTest, AbilityModifierScore20IsPlusFive) {
    Character c = makeCharacter();
    int score = 20;
    EXPECT_EQ(c.getAbilityModifier(score), 5);
}

TEST(CharacterTest, AbilityModifierScore1IsMinusFive) {
    Character c = makeCharacter();
    int score = 1;
    EXPECT_EQ(c.getAbilityModifier(score), -5);
}

TEST(CharacterTest, AbilityModifierScore16IsPlusThree) {
    Character c = makeCharacter();
    int score = 16;
    EXPECT_EQ(c.getAbilityModifier(score), 3);
}

// getAbilityModifier is static and takes its score by value, so it works on a
// const Character, with a literal, and without an instance at all.
TEST(CharacterTest, AbilityModifierIsCallableWithoutAnInstance) {
    EXPECT_EQ(Character::getAbilityModifier(10), 0);
    EXPECT_EQ(Character::getAbilityModifier(20), 5);
    EXPECT_EQ(Character::getAbilityModifier(1), -5);
    // Odd scores round down, matching floor((score - 10) / 2).
    EXPECT_EQ(Character::getAbilityModifier(9), -1);
    EXPECT_EQ(Character::getAbilityModifier(15), 2);
    EXPECT_EQ(Character::getAbilityModifier(7), -2);
}
