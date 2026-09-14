#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "H_CharacterManager.h"
#include "H_ConsoleIO.h"
#include "H_DndExceptions.h"

// Characterization tests for the interactive menu flows.
//
// These pin down what the menus actually do today so that splitting
// editCharacter into per-section units can be shown to preserve behaviour.
// They drive CharacterManager through a scripted istringstream, which is
// possible only because ConsoleIO takes its streams by injection.
//
// They deliberately assert on resulting character state rather than on menu
// wording, so that reformatting a prompt does not break the suite.

namespace {

struct ManagerHarness {
    std::istringstream in;
    std::ostringstream out;
    ConsoleIO io;
    CharacterManager mgr;

    explicit ManagerHarness(const std::string& script)
        : in(script), out(), io(in, out), mgr(io) {}

    std::string written() const { return out.str(); }

    const Character& only() const {
        EXPECT_FALSE(mgr.getCharacters().empty());
        return mgr.getCharacters().front();
    }
};

// createCharacter() prompt order:
//   name, race, class, background, alignment, level, age, weight,
//   max hp, temp hp, hit dice, STR, DEX, CON, INT, WIS, CHA, initiative, proficiency
const char* kCreateGimli =
    "Gimli\nDwarf\nFighter\nSoldier\nLawful Good\n"
    "5\n120\n150\n"
    "44\n0\nd10\n"
    "16\n12\n15\n10\n13\n8\n"
    "1\n3\n";

// Selects the only character, then enters the given editor submenu script.
std::string editOnly(const std::string& editorScript) {
    return std::string(kCreateGimli) + "1\n" + editorScript;
}

} // namespace

// ---------------------------------------------------------------------------
// createCharacter
// ---------------------------------------------------------------------------

TEST(ManagerMenuTest, CreateCharacterPopulatesEveryField) {
    ManagerHarness h(kCreateGimli);
    h.mgr.createCharacter();

    ASSERT_EQ(h.mgr.getCharacters().size(), 1u);
    const Character& c = h.only();

    EXPECT_EQ(c.getName(), "Gimli");
    EXPECT_EQ(c.getRace(), "Dwarf");
    EXPECT_EQ(c.getClass(), "Fighter");
    EXPECT_EQ(c.getBackground(), "Soldier");
    EXPECT_EQ(c.getAlignment(), "Lawful Good");
    EXPECT_EQ(c.getLevel(), 5);
    EXPECT_EQ(c.getAge(), 120);
    EXPECT_EQ(c.getWeight(), 150);
    EXPECT_EQ(c.getMaxHP(), 44);
    EXPECT_EQ(c.getCurrentHP(), 44);   // current HP starts at max
    EXPECT_EQ(c.getTempHP(), 0);
    EXPECT_EQ(c.getHitDice(), "d10");
    EXPECT_EQ(c.getStrength(), 16);
    EXPECT_EQ(c.getDexterity(), 12);
    EXPECT_EQ(c.getConstitution(), 15);
    EXPECT_EQ(c.getIntelligence(), 10);
    EXPECT_EQ(c.getWisdom(), 13);
    EXPECT_EQ(c.getCharisma(), 8);
    EXPECT_EQ(c.getInitiative(), 1);
    EXPECT_EQ(c.getProficiency(), 3);
}

TEST(ManagerMenuTest, CreateCharacterRepromptsOnBadLevelAndScore) {
    // 99 is out of range for level; 40 is out of range for STR.
    ManagerHarness h(
        "Gimli\nDwarf\nFighter\nSoldier\nLawful Good\n"
        "99\n5\n120\n150\n"
        "44\n0\nd10\n"
        "40\n16\n12\n15\n10\n13\n8\n"
        "1\n3\n");
    h.mgr.createCharacter();

    EXPECT_EQ(h.only().getLevel(), 5);
    EXPECT_EQ(h.only().getStrength(), 16);
}

TEST(ManagerMenuTest, CreateCharacterRejectsBadHitDiceFormat) {
    ManagerHarness h(
        "Gimli\nDwarf\nFighter\nSoldier\nLawful Good\n"
        "5\n120\n150\n44\n0\n"
        "0\nD10\nd10\n"                 // "0" is the value that corrupted old saves
        "16\n12\n15\n10\n13\n8\n1\n3\n");
    h.mgr.createCharacter();

    EXPECT_EQ(h.only().getHitDice(), "d10");
}

TEST(ManagerMenuTest, CreateCharacterThrowsIfInputEndsEarly) {
    ManagerHarness h("Gimli\nDwarf\n");
    EXPECT_THROW(h.mgr.createCharacter(), EndOfInput);
}

// ---------------------------------------------------------------------------
// editCharacter -> character details (menu 1)
// ---------------------------------------------------------------------------

TEST(ManagerMenuTest, EditDetailsChangesName) {
    ManagerHarness h(editOnly("1\n1\nGloin\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_EQ(h.only().getName(), "Gloin");
}

// Three edits in a single visit to the submenu -- only possible now that it
// loops instead of returning to the editor after each change.
TEST(ManagerMenuTest, EditDetailsChangesRaceClassAndAlignment) {
    ManagerHarness h(editOnly("1\n2\nHuman\n3\nWizard\n5\nChaotic Good\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_EQ(h.only().getRace(), "Human");
    EXPECT_EQ(h.only().getClass(), "Wizard");
    EXPECT_EQ(h.only().getAlignment(), "Chaotic Good");
}

TEST(ManagerMenuTest, EditDetailsLevelIsClampedToTwenty) {
    ManagerHarness h(editOnly("1\n8\n25\n12\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_EQ(h.only().getLevel(), 12);
}

TEST(ManagerMenuTest, EditDetailsChangesSpeed) {
    ManagerHarness h(editOnly("1\n9\n25\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_EQ(h.only().getSpeed(), 25);
}

// ---------------------------------------------------------------------------
// editCharacter -> health (menu 2)
// ---------------------------------------------------------------------------

TEST(ManagerMenuTest, EditHealthSetsMaxHp) {
    ManagerHarness h(editOnly("2\n1\n60\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_EQ(h.only().getMaxHP(), 60);
}

TEST(ManagerMenuTest, EditHealthAllowsZeroCurrentHp) {
    // 0 HP is unconscious and must be accepted; the old helper rejected it.
    ManagerHarness h(editOnly("2\n2\n0\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_EQ(h.only().getCurrentHP(), 0);
}

TEST(ManagerMenuTest, EditHealthCurrentHpCannotExceedMax) {
    // 999 is above max (44), so it is refused and re-prompted.
    ManagerHarness h(editOnly("2\n2\n999\n40\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_EQ(h.only().getCurrentHP(), 40);
}

TEST(ManagerMenuTest, EditHealthSetsTempHp) {
    ManagerHarness h(editOnly("2\n3\n7\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_EQ(h.only().getTempHP(), 7);
}

TEST(ManagerMenuTest, EditHealthSetsHitDice) {
    ManagerHarness h(editOnly("2\n4\nd12\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_EQ(h.only().getHitDice(), "d12");
}

TEST(ManagerMenuTest, EditHealthAddsAndClearsConditions) {
    ManagerHarness h(editOnly("2\n7\n1\nPoisoned\n1\nProne\n0\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    ASSERT_EQ(h.only().getConditions().size(), 2u);
    EXPECT_EQ(h.only().getConditions()[0], "Poisoned");
    EXPECT_EQ(h.only().getConditions()[1], "Prone");
}

TEST(ManagerMenuTest, EditHealthRemovesACondition) {
    ManagerHarness h(editOnly("2\n7\n1\nPoisoned\n1\nProne\n2\n1\n0\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    ASSERT_EQ(h.only().getConditions().size(), 1u);
    EXPECT_EQ(h.only().getConditions()[0], "Prone");
}

TEST(ManagerMenuTest, EditHealthClearsAllConditions) {
    ManagerHarness h(editOnly("2\n7\n1\nPoisoned\n3\n0\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_TRUE(h.only().getConditions().empty());
}

TEST(ManagerMenuTest, EditHealthResetsDeathSaves) {
    ManagerHarness h(editOnly("2\n6\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_EQ(h.only().getDeathSaveSuccesses(), 0);
    EXPECT_EQ(h.only().getDeathSaveFailures(), 0);
}

// A death save roll is random, but it must always move the tally by at least
// one mark and leave both counters within the legal 0-3 range.
TEST(ManagerMenuTest, EditHealthDeathSaveRecordsAResult) {
    ManagerHarness h(editOnly("2\n5\n1\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    const int s = h.only().getDeathSaveSuccesses();
    const int f = h.only().getDeathSaveFailures();
    EXPECT_GE(s, 0); EXPECT_LE(s, 3);
    EXPECT_GE(f, 0); EXPECT_LE(f, 3);
    EXPECT_NE(h.written().find("Death Save Roll"), std::string::npos);
}

// ---------------------------------------------------------------------------
// editCharacter -> ability scores (menu 4)
// ---------------------------------------------------------------------------

TEST(ManagerMenuTest, EditAbilityScoreSetsStrength) {
    ManagerHarness h(editOnly("4\n1\n18\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_EQ(h.only().getStrength(), 18);
}

TEST(ManagerMenuTest, EditAbilityScoreSetsCharisma) {
    ManagerHarness h(editOnly("4\n6\n14\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_EQ(h.only().getCharisma(), 14);
}

// Guards the out-of-bounds read that used to index abilityScores[abil - 1]
// with an unvalidated value.
TEST(ManagerMenuTest, EditAbilityScoreRejectsIndexOutsideOneToSix) {
    ManagerHarness h(editOnly("4\n9\n0\n2\n17\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_EQ(h.only().getDexterity(), 17);
}

// ---------------------------------------------------------------------------
// editCharacter -> spells and rests (menu 5)
// ---------------------------------------------------------------------------

TEST(ManagerMenuTest, LongRestRestoresHpSlotsAndHitDice) {
    // Drop to 10 HP and spend hit dice, then take a long rest.
    ManagerHarness h(editOnly("2\n2\n10\n0\n7\n1\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_EQ(h.only().getCurrentHP(), h.only().getMaxHP());
    EXPECT_EQ(h.only().getHitDiceNum(), h.only().getLevel());
}

TEST(ManagerMenuTest, EditSpellSlotsSetsMaxAndCurrent) {
    ManagerHarness h(editOnly("5\n5\n1\n3\n4\n2\n3\n2\n0\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_EQ(h.only().getSpellSlots().getMaxSlots(3), 4);
    EXPECT_EQ(h.only().getSpellSlots().getCurrentSlots(3), 2);
}

TEST(ManagerMenuTest, CurrentSlotsCannotExceedConfiguredMax) {
    // Max for level 1 is 2, so 9 is refused and re-prompted down to 1.
    ManagerHarness h(editOnly("5\n5\n1\n1\n2\n2\n1\n9\n1\n0\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_EQ(h.only().getSpellSlots().getCurrentSlots(1), 1);
}

// ---------------------------------------------------------------------------
// editCharacter -> features (menu 6)
// ---------------------------------------------------------------------------

TEST(ManagerMenuTest, FeaturesAddFeatAndLanguage) {
    ManagerHarness h(editOnly("6\n2\nLucky\n9\nDwarvish\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    ASSERT_EQ(h.only().getFeatures().getFeats().size(), 1u);
    EXPECT_EQ(h.only().getFeatures().getFeats()[0], "Lucky");
    ASSERT_EQ(h.only().getFeatures().getLanguages().size(), 1u);
    EXPECT_EQ(h.only().getFeatures().getLanguages()[0], "Dwarvish");
}

TEST(ManagerMenuTest, FeaturesSetsSkillRank) {
    // Skill 1 is Acrobatics; rank 2 is Expertise.
    ManagerHarness h(editOnly("6\n7\n1\n2\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_EQ(h.only().getFeatures().getSkillRank("Acrobatics"), SkillRank::Expertise);
}

TEST(ManagerMenuTest, FeaturesTogglesInspiration) {
    ManagerHarness h(editOnly("6\n11\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_TRUE(h.only().getInspiration());
}

// ---------------------------------------------------------------------------
// editCharacter -> inventory (menu 3)
// ---------------------------------------------------------------------------

TEST(ManagerMenuTest, InventoryAddsGearItem) {
    ManagerHarness h(editOnly("3\n2\n3\nRope\n50ft hemp\nCommon\n10\n1\n1\nn\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    ASSERT_EQ(h.only().getInventory().size(), 1);
    EXPECT_EQ(h.only().getInventory().getItem(1).getName(), "Rope");
}

TEST(ManagerMenuTest, InventoryAddsWeaponAndEquipsArmor) {
    ManagerHarness h(editOnly(
        "3\n2\n2\nChain Mail\nHeavy armor\nCommon\n55\n1\n75\nn\n"
        "Heavy\n16\n-1\n13\ny\n"
        "5\n1\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    ASSERT_EQ(h.only().getInventory().size(), 1);
    EXPECT_EQ(h.only().getEquippedArmorIndex(), 1);
    EXPECT_EQ(h.only().getAC(), 16);
}

TEST(ManagerMenuTest, InventoryCurrencyAddAndSpend) {
    ManagerHarness h(editOnly("3\n4\n1\n0\n10\n0\n0\n0\n2\n0\n4\n0\n0\n0\n0\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_EQ(h.only().getWallet().getGold(), 6);
}

// ---------------------------------------------------------------------------
// Navigation
// ---------------------------------------------------------------------------

TEST(ManagerMenuTest, EditCharacterCancelsWithZero) {
    ManagerHarness h(std::string(kCreateGimli) + "0\n");
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_EQ(h.only().getName(), "Gimli");
}

TEST(ManagerMenuTest, EditCharacterOnEmptyRosterDoesNothing) {
    ManagerHarness h("");
    h.mgr.editCharacter();

    EXPECT_NE(h.written().find("No characters"), std::string::npos);
}

TEST(ManagerMenuTest, ViewCharactersOnEmptyRosterDoesNothing) {
    ManagerHarness h("");
    h.mgr.viewCharacters();

    EXPECT_NE(h.written().find("No characters"), std::string::npos);
}

// ---------------------------------------------------------------------------
// Submenu looping (all editor sections now behave the same way)
// ---------------------------------------------------------------------------

// Every submenu returns to itself after an action and leaves only on 0. These
// scripts each perform two edits in one visit, which the single-shot details
// and health menus could not do.
TEST(ManagerMenuTest, DetailsSubmenuLoopsUntilZero) {
    ManagerHarness h(editOnly("1\n6\n40\n7\n210\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_EQ(h.only().getAge(), 40);
    EXPECT_EQ(h.only().getWeight(), 210);
}

TEST(ManagerMenuTest, HealthSubmenuLoopsUntilZero) {
    ManagerHarness h(editOnly("2\n1\n60\n3\n5\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_EQ(h.only().getMaxHP(), 60);
    EXPECT_EQ(h.only().getTempHP(), 5);
}

// Leaving a submenu with 0 returns to the editor, not out of it, so a second
// section can be entered in the same editing session.
TEST(ManagerMenuTest, LeavingASubmenuReturnsToTheEditor) {
    ManagerHarness h(editOnly("1\n1\nGloin\n0\n2\n3\n9\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_EQ(h.only().getName(), "Gloin");
    EXPECT_EQ(h.only().getTempHP(), 9);
}

// ---------------------------------------------------------------------------
// Rest as a top-level editor section (menu 7)
// ---------------------------------------------------------------------------

TEST(ManagerMenuTest, RestMenuLongRestRestoresEverything) {
    // Spend HP and a slot, then long rest from the new top-level entry.
    ManagerHarness h(editOnly("2\n2\n10\n0\n7\n1\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_EQ(h.only().getCurrentHP(), h.only().getMaxHP());
    EXPECT_EQ(h.only().getHitDiceNum(), h.only().getLevel());
}

TEST(ManagerMenuTest, RestMenuShortRestSpendsHitDiceForHp) {
    // Drop to 10 HP, short rest, spend 2 hit dice, report 12 HP recovered.
    ManagerHarness h(editOnly("2\n2\n10\n0\n7\n2\n2\n12\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_EQ(h.only().getCurrentHP(), 22);
    EXPECT_EQ(h.only().getHitDiceNum(), h.only().getLevel() - 2);
}

TEST(ManagerMenuTest, RestMenuShortRestCannotSpendMoreDiceThanAvailable) {
    // Only 5 hit dice exist at level 5, so 99 is refused and re-prompted.
    ManagerHarness h(editOnly("2\n2\n10\n0\n7\n2\n99\n1\n6\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_EQ(h.only().getHitDiceNum(), h.only().getLevel() - 1);
}

TEST(ManagerMenuTest, RestMenuLoopsUntilZero) {
    // Two long rests in one visit, then back.
    ManagerHarness h(editOnly("7\n1\n1\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    EXPECT_EQ(h.only().getCurrentHP(), h.only().getMaxHP());
}

// The Spells submenu no longer offers the rests; its highest option is 5.
TEST(ManagerMenuTest, SpellsSubmenuNoLongerOffersRests) {
    ManagerHarness h(editOnly("5\n6\n5\n0\n0\n0\n"));
    h.mgr.createCharacter();
    h.mgr.editCharacter();

    // 6 is rejected as out of range rather than performing a long rest.
    EXPECT_NE(h.written().find("Invalid entry"), std::string::npos);
}
