#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "H_Character.h"
#include "H_DndExceptions.h"

// Tests for the versioned character.txt format and its validating loader.
//
// The loader exists because the original one read fields positionally with no
// version marker and no validation, so a value that would be refused at a
// prompt was accepted without comment from a file. Three saved characters
// carried hit_dice "0", charisma 0 and proficiency -1 for months as a result.

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
