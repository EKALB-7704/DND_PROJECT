#include <gtest/gtest.h>
#include "H_Spell.h"
#include "H_SpellBook.h"

// ── Spell helpers ─────────────────────────────────────────────────────────────

static Spell makeFireball() {
    return Spell(
        "Fireball", "Evocation", "Damage",
        /*level*/ 3, "1 action", "150 ft", "V, S, M",
        "Instantaneous", "Dexterity", "A ball of fire explodes at a point."
    );
}

static Spell makeMagicMissile() {
    return Spell(
        "Magic Missile", "Evocation", "Damage",
        /*level*/ 1, "1 action", "120 ft", "V, S",
        "Instantaneous", "None", "Unerring bolts of force strike the target."
    );
}

static Spell makeMageHand() {
    return Spell(
        "Mage Hand", "Conjuration", "Utility",
        /*level*/ 0, "1 action", "30 ft", "V, S",
        "1 minute", "None", "A spectral floating hand appears."
    );
}

// ── Spell: default constructor ─────────────────────────────────────────────────

TEST(SpellTest, DefaultConstructorEmptyStrings) {
    Spell s;
    EXPECT_EQ(s.getSpellName(),        "");
    EXPECT_EQ(s.getSpellType(),        "");
    EXPECT_EQ(s.getSpellEffect(),      "");
    EXPECT_EQ(s.getSpellTime(),        "");
    EXPECT_EQ(s.getSpellRange(),       "");
    EXPECT_EQ(s.getSpellComponents(),  "");
    EXPECT_EQ(s.getSpellDuration(),    "");
    EXPECT_EQ(s.getSpellSavingThrow(), "");
    EXPECT_EQ(s.getSpellDescription(), "");
}

TEST(SpellTest, DefaultConstructorLevelIsZero) {
    Spell s;
    EXPECT_EQ(s.getSpellLevel(), 0);
}

// ── Spell: parametrized constructor ───────────────────────────────────────────

TEST(SpellTest, ParametrizedConstructorSetsAllStringFields) {
    Spell s = makeFireball();
    EXPECT_EQ(s.getSpellName(),        "Fireball");
    EXPECT_EQ(s.getSpellType(),        "Evocation");
    EXPECT_EQ(s.getSpellEffect(),      "Damage");
    EXPECT_EQ(s.getSpellTime(),        "1 action");
    EXPECT_EQ(s.getSpellRange(),       "150 ft");
    EXPECT_EQ(s.getSpellComponents(),  "V, S, M");
    EXPECT_EQ(s.getSpellDuration(),    "Instantaneous");
    EXPECT_EQ(s.getSpellSavingThrow(), "Dexterity");
    EXPECT_EQ(s.getSpellDescription(), "A ball of fire explodes at a point.");
}

TEST(SpellTest, ParametrizedConstructorSetsLevel) {
    EXPECT_EQ(makeFireball().getSpellLevel(),      3);
    EXPECT_EQ(makeMagicMissile().getSpellLevel(),  1);
}

TEST(SpellTest, CantripLevelIsZero) {
    EXPECT_EQ(makeMageHand().getSpellLevel(), 0);
}

// ── Spell: setters ────────────────────────────────────────────────────────────

TEST(SpellTest, SetSpellNameUpdatesName) {
    Spell s;
    s.setSpellName("Cure Wounds");
    EXPECT_EQ(s.getSpellName(), "Cure Wounds");
}

TEST(SpellTest, SetSpellLevelUpdatesLevel) {
    Spell s;
    s.setSpellLevel(9);
    EXPECT_EQ(s.getSpellLevel(), 9);
}

TEST(SpellTest, SetAllFieldsViaSetters) {
    Spell s;
    s.setSpellName("Shield");
    s.setSpellType("Abjuration");
    s.setSpellEffect("Defense");
    s.setSpellLevel(1);
    s.setSpellTime("1 reaction");
    s.setSpellRange("Self");
    s.setSpellComponents("V, S");
    s.setSpellDuration("1 round");
    s.setSpellSavingThrow("None");
    s.setSpellDescription("An invisible barrier of force appears.");

    EXPECT_EQ(s.getSpellName(),        "Shield");
    EXPECT_EQ(s.getSpellType(),        "Abjuration");
    EXPECT_EQ(s.getSpellEffect(),      "Defense");
    EXPECT_EQ(s.getSpellLevel(),       1);
    EXPECT_EQ(s.getSpellTime(),        "1 reaction");
    EXPECT_EQ(s.getSpellRange(),       "Self");
    EXPECT_EQ(s.getSpellComponents(),  "V, S");
    EXPECT_EQ(s.getSpellDuration(),    "1 round");
    EXPECT_EQ(s.getSpellSavingThrow(), "None");
    EXPECT_EQ(s.getSpellDescription(), "An invisible barrier of force appears.");
}

// ── SpellBook: empty state ─────────────────────────────────────────────────────

TEST(SpellBookTest, StartsEmpty) {
    SpellBook sb;
    EXPECT_TRUE(sb.getAllSpells().empty());
}

TEST(SpellBookTest, GetSpellsByLevelOnEmptyBookReturnsEmpty) {
    SpellBook sb;
    EXPECT_TRUE(sb.getSpellsByLevel(1).empty());
}

// ── SpellBook: addSpell ────────────────────────────────────────────────────────

TEST(SpellBookTest, AddSpellIncreasesCount) {
    SpellBook sb;
    sb.addSpell(makeFireball());
    EXPECT_EQ(sb.getAllSpells().size(), 1u);
}

TEST(SpellBookTest, AddMultipleSpellsAccumulates) {
    SpellBook sb;
    sb.addSpell(makeFireball());
    sb.addSpell(makeMagicMissile());
    sb.addSpell(makeMageHand());
    EXPECT_EQ(sb.getAllSpells().size(), 3u);
}

TEST(SpellBookTest, AddedSpellNameIsPreserved) {
    SpellBook sb;
    sb.addSpell(makeFireball());
    EXPECT_EQ(sb.getAllSpells()[0].getSpellName(), "Fireball");
}

// ── SpellBook: removeSpell ─────────────────────────────────────────────────────

TEST(SpellBookTest, RemoveSpellDecreasesCount) {
    SpellBook sb;
    sb.addSpell(makeFireball());
    sb.addSpell(makeMagicMissile());
    sb.removeSpell("Fireball");
    EXPECT_EQ(sb.getAllSpells().size(), 1u);
}

TEST(SpellBookTest, RemoveSpellRemovesCorrectSpell) {
    SpellBook sb;
    sb.addSpell(makeFireball());
    sb.addSpell(makeMagicMissile());
    sb.removeSpell("Fireball");
    EXPECT_EQ(sb.getAllSpells()[0].getSpellName(), "Magic Missile");
}

TEST(SpellBookTest, RemoveOnlyRemainingSpellLeavesBookEmpty) {
    SpellBook sb;
    sb.addSpell(makeFireball());
    sb.removeSpell("Fireball");
    EXPECT_TRUE(sb.getAllSpells().empty());
}

TEST(SpellBookTest, RemoveNonExistentSpellDoesNotCrash) {
    SpellBook sb;
    sb.addSpell(makeFireball());
    EXPECT_NO_FATAL_FAILURE(sb.removeSpell("Nonexistent Spell"));
    EXPECT_EQ(sb.getAllSpells().size(), 1u);  // unchanged
}

// ── SpellBook: getSpellsByLevel ────────────────────────────────────────────────

TEST(SpellBookTest, GetSpellsByLevelFiltersToCorrectLevel) {
    SpellBook sb;
    sb.addSpell(makeMageHand());      // level 0
    sb.addSpell(makeMagicMissile());  // level 1
    sb.addSpell(makeFireball());      // level 3

    auto level1 = sb.getSpellsByLevel(1);
    ASSERT_EQ(level1.size(), 1u);
    EXPECT_EQ(level1[0].getSpellName(), "Magic Missile");
}

TEST(SpellBookTest, GetSpellsByLevelReturnsCantrips) {
    SpellBook sb;
    sb.addSpell(makeMageHand());
    sb.addSpell(makeFireball());

    auto cantrips = sb.getSpellsByLevel(0);
    ASSERT_EQ(cantrips.size(), 1u);
    EXPECT_EQ(cantrips[0].getSpellName(), "Mage Hand");
}

TEST(SpellBookTest, GetSpellsByLevelWithNoMatchReturnsEmpty) {
    SpellBook sb;
    sb.addSpell(makeFireball());   // level 3
    EXPECT_TRUE(sb.getSpellsByLevel(9).empty());
}

TEST(SpellBookTest, GetSpellsByLevelReturnsMultipleMatchingSpells) {
    SpellBook sb;
    sb.addSpell(makeMagicMissile());  // level 1
    Spell shield("Shield", "Abjuration", "Defense", 1,
                 "1 reaction", "Self", "V, S", "1 round", "None", "Barrier of force.");
    sb.addSpell(shield);

    auto level1 = sb.getSpellsByLevel(1);
    EXPECT_EQ(level1.size(), 2u);
}

TEST(SpellBookTest, GetAllSpellsReturnsEveryAddedSpell) {
    SpellBook sb;
    sb.addSpell(makeFireball());
    sb.addSpell(makeMagicMissile());

    auto all = sb.getAllSpells();
    ASSERT_EQ(all.size(), 2u);
    EXPECT_EQ(all[0].getSpellName(), "Fireball");
    EXPECT_EQ(all[1].getSpellName(), "Magic Missile");
}

TEST(SpellBookTest, UpdateSpellReplacesSelectedSpell) {
    SpellBook sb;
    sb.addSpell(makeFireball());
    sb.addSpell(makeMagicMissile());

    Spell updated = makeMagicMissile();
    updated.setSpellName("Shield");
    updated.setSpellLevel(1);

    EXPECT_TRUE(sb.updateSpell(0, updated));

    auto all = sb.getAllSpells();
    ASSERT_EQ(all.size(), 2u);
    EXPECT_EQ(all[0].getSpellName(), "Shield");
    EXPECT_EQ(all[0].getSpellLevel(), 1);
    EXPECT_EQ(all[1].getSpellName(), "Magic Missile");
}

TEST(SpellBookTest, UpdateSpellRejectsOutOfRangeIndex) {
    SpellBook sb;
    sb.addSpell(makeFireball());

    EXPECT_FALSE(sb.updateSpell(3, makeMagicMissile()));
    ASSERT_EQ(sb.getAllSpells().size(), 1u);
    EXPECT_EQ(sb.getAllSpells()[0].getSpellName(), "Fireball");
}
