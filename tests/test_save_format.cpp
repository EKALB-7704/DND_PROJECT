#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "H_Character.h"
#include "H_CharacterFeatures.h"
#include "H_DndExceptions.h"
#include "H_Gear.h"
#include "H_Inventory.h"
#include "H_SpellBook.h"
#include "H_SpellSlots.h"
#include "H_Wallet.h"
#include <memory>

// Tests for the versioned character.txt format and its validating loader.
//
// The loader exists because the original one read fields positionally with no
// version marker and no validation, so a value that would be refused at a
// prompt was accepted without comment from a file. Three saved characters
// carried hitDice "0", charisma 0 and proficiency -1 for months as a result.

namespace {

namespace fs = std::filesystem;

// Creates a temp character directory and removes it on destruction.
struct TempCharDir {
    fs::path dir;

    explicit TempCharDir(const std::string& name)
        : dir(fs::temp_directory_path() / ("dnd_save_test_" + name)) {
        fs::remove_all(dir);
        fs::create_directories(dir);
    }
    ~TempCharDir() { fs::remove_all(dir); }

    void writeCharacterTxt(const std::string& contents) const {
        std::ofstream f(dir / "character.txt");
        f << contents;
    }

    std::string readCharacterTxt() const {
        std::ifstream f(dir / "character.txt");
        return std::string((std::istreambuf_iterator<char>(f)),
                           std::istreambuf_iterator<char>());
    }

    std::string path() const { return dir.string(); }
};

// A valid v0 (headerless) file, matching what older builds wrote.
const char* kLegacyGood =
    "Thrain\nDwarf\nFighter\nSoldier\nLawful Good\n"
    "10 120 232\n"
    "102 102 1\n"
    "0 0\n"
    "d10 10\n"
    "20 16 16 11 12 12 3 4\n"
    "2 -1\n"
    "1\n"
    "30\n"
    "0\n";

// The exact corruption found in the saved characters: hit dice "0",
// charisma 0, proficiency -1.
const char* kLegacyCorrupt =
    "paul\nhuman\nwarlock\nchaotic neutral\nlawful good\n"
    "5 21 200\n"
    "66 66 0\n"
    "0 0\n"
    "0 5\n"
    "10 10 10 10 10 0 2 -1\n"
    "-1 0\n"
    "1\n"
    "30\n"
    "0\n";

Character load(const TempCharDir& d, std::vector<std::string>* repairs = nullptr) {
    return Character::loadFromDirectory(d.path(), repairs);
}

} // namespace

// ---------------------------------------------------------------------------
// Version header
// ---------------------------------------------------------------------------

TEST(SaveFormatTest, SaveWritesVersionHeader) {
    TempCharDir d("header");
    Character c("Gimli", "Dwarf", "Fighter", "Soldier", "Lawful Good",
                5, 120, 150, 44, 44, 0, "d10", 16, 12, 15, 10, 13, 8, 1, 3);
    c.saveToDirectory(d.path());

    const std::string text = d.readCharacterTxt();
    EXPECT_EQ(text.rfind("#DNDCHAR ", 0), 0u);
    EXPECT_NE(text.find("#DNDCHAR " + std::to_string(Character::kSaveFormatVersion)),
              std::string::npos);
}

TEST(SaveFormatTest, VersionedRoundtripPreservesFields) {
    TempCharDir d("roundtrip");
    Character c("Gimli", "Dwarf", "Fighter", "Soldier", "Lawful Good",
                5, 120, 150, 40, 44, 3, "d10", 16, 12, 15, 10, 13, 8, 1, 3);
    c.addCondition("Poisoned");
    c.saveToDirectory(d.path());

    std::vector<std::string> repairs;
    const Character back = load(d, &repairs);

    EXPECT_TRUE(repairs.empty()) << "a file we just wrote should need no repair";
    EXPECT_EQ(back.getName(), "Gimli");
    EXPECT_EQ(back.getLevel(), 5);
    EXPECT_EQ(back.getCurrentHP(), 40);
    EXPECT_EQ(back.getMaxHP(), 44);
    EXPECT_EQ(back.getTempHP(), 3);
    EXPECT_EQ(back.getHitDice(), "d10");
    EXPECT_EQ(back.getCharisma(), 8);
    EXPECT_EQ(back.getProficiency(), 3);
    ASSERT_EQ(back.getConditions().size(), 1u);
    EXPECT_EQ(back.getConditions()[0], "Poisoned");
}

TEST(SaveFormatTest, RejectsFutureVersion) {
    TempCharDir d("future");
    d.writeCharacterTxt("#DNDCHAR 99\n" + std::string(kLegacyGood));

    EXPECT_THROW(load(d), LoadError);
}

TEST(SaveFormatTest, RejectsUnreadableVersionHeader) {
    TempCharDir d("badheader");
    d.writeCharacterTxt("#DNDCHAR banana\n" + std::string(kLegacyGood));

    EXPECT_THROW(load(d), LoadError);
}

// ---------------------------------------------------------------------------
// Legacy (v0, headerless) files
// ---------------------------------------------------------------------------

TEST(SaveFormatTest, ReadsLegacyFileWithoutHeader) {
    TempCharDir d("legacy");
    d.writeCharacterTxt(kLegacyGood);

    std::vector<std::string> repairs;
    const Character c = load(d, &repairs);

    EXPECT_TRUE(repairs.empty()) << "a valid legacy file needs no repair";
    EXPECT_EQ(c.getName(), "Thrain");
    EXPECT_EQ(c.getLevel(), 10);
    EXPECT_EQ(c.getHitDice(), "d10");
    EXPECT_EQ(c.getStrength(), 20);
    EXPECT_EQ(c.getProficiency(), 4);
}

TEST(SaveFormatTest, LegacyFileIsUpgradedOnSave) {
    TempCharDir d("upgrade");
    d.writeCharacterTxt(kLegacyGood);

    const Character c = load(d);
    c.saveToDirectory(d.path());

    EXPECT_EQ(d.readCharacterTxt().rfind("#DNDCHAR ", 0), 0u);
}

// ---------------------------------------------------------------------------
// Repair of the real corruption
// ---------------------------------------------------------------------------

TEST(SaveFormatTest, RepairsTheCorruptionFoundInSavedCharacters) {
    TempCharDir d("corrupt");
    d.writeCharacterTxt(kLegacyCorrupt);

    std::vector<std::string> repairs;
    const Character c = load(d, &repairs);

    // Bad values are replaced with legal ones rather than silently kept.
    EXPECT_EQ(c.getHitDice(), "d8");
    EXPECT_EQ(c.getCharisma(), 10);
    EXPECT_EQ(c.getProficiency(), 2);

    // Untouched fields survive intact.
    EXPECT_EQ(c.getName(), "paul");
    EXPECT_EQ(c.getLevel(), 5);
    EXPECT_EQ(c.getMaxHP(), 66);

    EXPECT_EQ(repairs.size(), 3u);
}

TEST(SaveFormatTest, RepairsAreReportedNotSilent) {
    TempCharDir d("reported");
    d.writeCharacterTxt(kLegacyCorrupt);

    std::vector<std::string> repairs;
    load(d, &repairs);

    const std::string all = [&] {
        std::string s;
        for (const auto& r : repairs) s += r + "\n";
        return s;
    }();
    EXPECT_NE(all.find("hit dice"), std::string::npos);
    EXPECT_NE(all.find("charisma"), std::string::npos);
    EXPECT_NE(all.find("proficiency"), std::string::npos);
}

TEST(SaveFormatTest, RepairsSurviveASaveLoadCycle) {
    TempCharDir d("persist");
    d.writeCharacterTxt(kLegacyCorrupt);

    load(d).saveToDirectory(d.path());

    std::vector<std::string> repairs;
    const Character again = load(d, &repairs);
    EXPECT_TRUE(repairs.empty()) << "re-saved data should already be valid";
    EXPECT_EQ(again.getHitDice(), "d8");
    EXPECT_EQ(again.getProficiency(), 2);
}

// A null repairs pointer must still repair, just without reporting.
TEST(SaveFormatTest, RepairWorksWithoutARepairsVector) {
    TempCharDir d("norepairvec");
    d.writeCharacterTxt(kLegacyCorrupt);

    const Character c = Character::loadFromDirectory(d.path());
    EXPECT_EQ(c.getHitDice(), "d8");
}

// ---------------------------------------------------------------------------
// Field-level validation
// ---------------------------------------------------------------------------

TEST(SaveFormatTest, ClampsCurrentHpAboveMax) {
    TempCharDir d("hp");
    d.writeCharacterTxt(
        "X\nHuman\nFighter\nSoldier\nLawful Good\n"
        "5 20 150\n999 44 0\n0 0\nd10 5\n"
        "10 10 10 10 10 10 0 2\n-1 -1\n0\n30\n0\n");

    std::vector<std::string> repairs;
    const Character c = load(d, &repairs);

    EXPECT_EQ(c.getMaxHP(), 44);
    EXPECT_EQ(c.getCurrentHP(), 44);
    EXPECT_FALSE(repairs.empty());
}

TEST(SaveFormatTest, ClampsOutOfRangeLevelAndDeathSaves) {
    TempCharDir d("ranges");
    d.writeCharacterTxt(
        "X\nHuman\nFighter\nSoldier\nLawful Good\n"
        "77 20 150\n44 44 0\n9 9\nd10 5\n"
        "10 10 10 10 10 10 0 2\n-1 -1\n0\n30\n0\n");

    const Character c = load(d);

    EXPECT_GE(c.getLevel(), 1);
    EXPECT_LE(c.getLevel(), 20);
    EXPECT_LE(c.getDeathSaveSuccesses(), 3);
    EXPECT_LE(c.getDeathSaveFailures(), 3);
}

// ---------------------------------------------------------------------------
// Malformed files are refused rather than half-read
// ---------------------------------------------------------------------------

TEST(SaveFormatTest, ThrowsOnMissingFile) {
    TempCharDir d("missing");   // directory exists, character.txt does not
    EXPECT_THROW(load(d), LoadError);
}

TEST(SaveFormatTest, ThrowsOnTruncatedFile) {
    TempCharDir d("truncated");
    d.writeCharacterTxt("X\nHuman\nFighter\n");

    EXPECT_THROW(load(d), LoadError);
}

TEST(SaveFormatTest, ThrowsOnEmptyName) {
    TempCharDir d("noname");
    d.writeCharacterTxt(
        "\nHuman\nFighter\nSoldier\nLawful Good\n"
        "5 20 150\n44 44 0\n0 0\nd10 5\n"
        "10 10 10 10 10 10 0 2\n-1 -1\n0\n30\n0\n");

    EXPECT_THROW(load(d), LoadError);
}

// The stat line holding the wrong number of fields is exactly how a mid-project
// field addition used to shift every later value.
TEST(SaveFormatTest, ThrowsOnWrongFieldCount) {
    TempCharDir d("shortline");
    d.writeCharacterTxt(
        "X\nHuman\nFighter\nSoldier\nLawful Good\n"
        "5 20 150\n44 44 0\n0 0\nd10 5\n"
        "10 10 10\n-1 -1\n0\n30\n0\n");

    EXPECT_THROW(load(d), LoadError);
}

TEST(SaveFormatTest, ThrowsOnNonNumericField) {
    TempCharDir d("nonnumeric");
    d.writeCharacterTxt(
        "X\nHuman\nFighter\nSoldier\nLawful Good\n"
        "five 20 150\n44 44 0\n0 0\nd10 5\n"
        "10 10 10 10 10 10 0 2\n-1 -1\n0\n30\n0\n");

    EXPECT_THROW(load(d), LoadError);
}

// ===========================================================================
// Sub-file versioning (features, inventory, wallet, spellslots, spells)
//
// Phase 3b: these five carried the same weaknesses character.txt did --
// no version marker, no validation, and unbounded record counts.
// ===========================================================================

namespace {

// Writes `contents` to a temp file and hands back the path.
struct TempSaveFile {
    fs::path path;

    TempSaveFile(const std::string& name, const std::string& contents)
        : path(fs::temp_directory_path() / ("dnd_subfile_" + name)) {
        std::ofstream f(path);
        f << contents;
    }
    ~TempSaveFile() { fs::remove(path); }

    std::string read() const {
        std::ifstream f(path);
        return std::string((std::istreambuf_iterator<char>(f)),
                           std::istreambuf_iterator<char>());
    }
    std::string str() const { return path.string(); }
};

} // namespace

// --- Wallet ----------------------------------------------------------------

TEST(SubFileFormatTest, WalletWritesHeaderAndRoundtrips) {
    TempSaveFile t("wallet_rt", "");
    {
        Wallet w;
        w.adjustGold(12);
        w.adjustCopper(7);
        std::ofstream out(t.path);
        w.save(out);
    }
    EXPECT_EQ(t.read().rfind("#DNDWALLET ", 0), 0u);

    Wallet back;
    std::ifstream in(t.path);
    back.load(in);
    EXPECT_EQ(back.getGold(), 12);
    EXPECT_EQ(back.getCopper(), 7);
}

TEST(SubFileFormatTest, WalletReadsLegacyHeaderlessFile) {
    TempSaveFile t("wallet_legacy", "7 0 0 12 0\n");
    Wallet w;
    std::ifstream in(t.path);
    w.load(in);

    EXPECT_EQ(w.getCopper(), 7);
    EXPECT_EQ(w.getGold(), 12);
}

TEST(SubFileFormatTest, WalletRejectsShortLine) {
    TempSaveFile t("wallet_short", "1 2 3\n");
    Wallet w;
    std::ifstream in(t.path);
    EXPECT_THROW(w.load(in), LoadError);
}

TEST(SubFileFormatTest, WalletRejectsNonNumeric) {
    TempSaveFile t("wallet_junk", "1 2 three 4 5\n");
    Wallet w;
    std::ifstream in(t.path);
    EXPECT_THROW(w.load(in), LoadError);
}

TEST(SubFileFormatTest, WalletClampsNegativeAmounts) {
    TempSaveFile t("wallet_neg", "-5 0 0 -3 0\n");
    Wallet w;
    std::ifstream in(t.path);
    w.load(in);

    EXPECT_EQ(w.getCopper(), 0);
    EXPECT_EQ(w.getGold(), 0);
}

// --- SpellSlots ------------------------------------------------------------

TEST(SubFileFormatTest, SpellSlotsWritesHeaderAndRoundtrips) {
    TempSaveFile t("slots_rt", "");
    {
        SpellSlots s;
        s.setSlots(1, 4);
        s.setCurrentSlots(1, 2);
        s.setSlots(3, 2);
        std::ofstream out(t.path);
        s.save(out);
    }
    EXPECT_EQ(t.read().rfind("#DNDSLOTS ", 0), 0u);

    SpellSlots back;
    std::ifstream in(t.path);
    back.load(in);
    EXPECT_EQ(back.getMaxSlots(1), 4);
    EXPECT_EQ(back.getCurrentSlots(1), 2);
    EXPECT_EQ(back.getMaxSlots(3), 2);
}

TEST(SubFileFormatTest, SpellSlotsReadsLegacyHeaderlessFile) {
    TempSaveFile t("slots_legacy", "1\n2 3 1\n");
    SpellSlots s;
    std::ifstream in(t.path);
    s.load(in);

    EXPECT_EQ(s.getMaxSlots(2), 3);
    EXPECT_EQ(s.getCurrentSlots(2), 1);
}

TEST(SubFileFormatTest, SpellSlotsRejectsImplausibleCount) {
    TempSaveFile t("slots_count", "999999999\n");
    SpellSlots s;
    std::ifstream in(t.path);
    EXPECT_THROW(s.load(in), LoadError);
}

TEST(SubFileFormatTest, SpellSlotsSkipsOutOfRangeLevels) {
    // Level 0 and 12 are not slot pools; level 2 is kept.
    TempSaveFile t("slots_badlevel", "3\n0 4 4\n12 4 4\n2 3 3\n");
    SpellSlots s;
    std::ifstream in(t.path);
    s.load(in);

    EXPECT_EQ(s.getMaxSlots(2), 3);
    EXPECT_EQ(s.getMaxSlots(0), 0);
    EXPECT_EQ(s.getMaxSlots(12), 0);
}

TEST(SubFileFormatTest, SpellSlotsClampsCurrentAboveMax) {
    TempSaveFile t("slots_over", "1\n1 2 9\n");
    SpellSlots s;
    std::ifstream in(t.path);
    s.load(in);

    EXPECT_EQ(s.getCurrentSlots(1), 2);
}

// --- Inventory -------------------------------------------------------------

TEST(SubFileFormatTest, InventoryWritesHeaderAndRoundtrips) {
    TempSaveFile t("inv_rt", "");
    {
        Inventory inv;
        inv.addItem(std::make_unique<Gear>("Rope", "50ft", 10.0f, 1, 1, "Common", false));
        std::ofstream out(t.path);
        inv.save(out);
    }
    EXPECT_EQ(t.read().rfind("#DNDINV ", 0), 0u);

    Inventory back;
    std::ifstream in(t.path);
    back.load(in);
    ASSERT_EQ(back.size(), 1);
    EXPECT_EQ(back.getItem(1).getName(), "Rope");
}

TEST(SubFileFormatTest, InventoryRejectsImplausibleCount) {
    TempSaveFile t("inv_count", "999999999\n");
    Inventory inv;
    std::ifstream in(t.path);
    EXPECT_THROW(inv.load(in), LoadError);
}

TEST(SubFileFormatTest, InventoryRejectsNonNumericCount) {
    TempSaveFile t("inv_junk", "lots\n");
    Inventory inv;
    std::ifstream in(t.path);
    EXPECT_THROW(inv.load(in), LoadError);
}

// --- CharacterFeatures -----------------------------------------------------

TEST(SubFileFormatTest, FeaturesWritesHeaderAndRoundtrips) {
    TempSaveFile t("feat_rt", "");
    {
        CharacterFeatures cf;
        cf.addFeat("Lucky");
        cf.addLanguage("Dwarvish");
        cf.setSkillRank("Stealth", SkillRank::Expertise);
        cf.setSaveProficiency("DEX", true);
        std::ofstream out(t.path);
        cf.save(out);
    }
    EXPECT_EQ(t.read().rfind("#DNDFEATS ", 0), 0u);

    CharacterFeatures back;
    std::ifstream in(t.path);
    back.load(in);
    ASSERT_EQ(back.getFeats().size(), 1u);
    EXPECT_EQ(back.getFeats()[0], "Lucky");
    ASSERT_EQ(back.getLanguages().size(), 1u);
    EXPECT_EQ(back.getLanguages()[0], "Dwarvish");
    EXPECT_EQ(back.getSkillRank("Stealth"), SkillRank::Expertise);
    EXPECT_TRUE(back.getSaveProficiency("DEX"));
}

TEST(SubFileFormatTest, FeaturesRejectsImplausibleCount) {
    TempSaveFile t("feat_count", "999999999\n");
    CharacterFeatures cf;
    std::ifstream in(t.path);
    EXPECT_THROW(cf.load(in), LoadError);
}

TEST(SubFileFormatTest, FeaturesRejectsTruncatedList) {
    // Claims three feats but supplies one.
    TempSaveFile t("feat_trunc", "3\nLucky\n");
    CharacterFeatures cf;
    std::ifstream in(t.path);
    EXPECT_THROW(cf.load(in), LoadError);
}

TEST(SubFileFormatTest, FeaturesTreatsOutOfRangeSkillRankAsUntrained) {
    TempSaveFile t("feat_rank", "0\n0\n1\nStealth\nDEX\n7\n");
    CharacterFeatures cf;
    std::ifstream in(t.path);
    cf.load(in);

    EXPECT_EQ(cf.getSkillRank("Stealth"), SkillRank::None);
}

// --- SpellBook -------------------------------------------------------------

TEST(SubFileFormatTest, SpellBookWritesHeaderAndRoundtrips) {
    TempSaveFile t("book_rt", "");
    {
        SpellBook b;
        b.addSpell(Spell("Shield", "Abjuration", "Block", 1, "1 reaction",
                         "Self", "V,S", "1 round", "None", "A shield"));
        b.saveSpellBook(t.str());
    }
    EXPECT_EQ(t.read().rfind("#DNDSPELLS ", 0), 0u);

    SpellBook back;
    back.loadSpellBook(t.str());
    ASSERT_EQ(back.getAllSpells().size(), 1u);
    EXPECT_EQ(back.getAllSpells()[0].getSpellName(), "Shield");
}

TEST(SubFileFormatTest, SpellBookRejectsImplausibleCount) {
    TempSaveFile t("book_count", "999999999\n");
    SpellBook b;
    EXPECT_THROW(b.loadSpellBook(t.str()), LoadError);
}

// The slot and spellbook tags share a "#DNDS" prefix; the trailing space in
// the header match is what keeps one from being read as the other.
TEST(SubFileFormatTest, SimilarTagsAreNotConfused) {
    TempSaveFile t("tagmix", "#DNDSPELLS 1\n0\n");
    SpellSlots s;
    std::ifstream in(t.path);
    // Read as a headerless slot file, whose first line is not a valid count.
    EXPECT_THROW(s.load(in), LoadError);
}
