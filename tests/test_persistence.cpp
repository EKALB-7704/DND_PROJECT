#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>
#include <memory>
#include <filesystem>
#include "H_Item.h"
#include "H_Weapon.h"
#include "H_Armor.h"
#include "H_Gear.h"
#include "H_spells.h"
#include "H_SpellBook.h"
#include "H_Character.h"

// RAII wrapper: removes a temp file on destruction, even if the test throws
struct TempFile {
    const char* path;
    explicit TempFile(const char* p) : path(p) { std::remove(p); }
    ~TempFile() { std::remove(path); }
};

// ── Weapon save/load roundtrip ─────────────────────────────────────────────────

TEST(PersistenceTest, WeaponRoundtrip) {
    TempFile tmp("dnd_test_weapon.tmp");

    {
        Weapon w("Dagger", "A small blade", 1.0f, 1, 2, "Common", false,
                 "1d4", "Piercing", "Simple", "Melee", "Finesse, Light, Thrown", "20/60 ft");
        std::ofstream out(tmp.path);
        ASSERT_TRUE(out.is_open());
        w.save(out);
    }

    {
        std::ifstream in(tmp.path);
        ASSERT_TRUE(in.is_open());
        auto item = Item::loadFromFile(in);
        ASSERT_NE(item, nullptr);

        // Base fields preserved
        EXPECT_EQ(item->getName(),        "Dagger");
        EXPECT_EQ(item->getDescription(), "A small blade");
        EXPECT_FLOAT_EQ(item->getWeight(), 1.0f);
        EXPECT_EQ(item->getQuantity(), 1);
        EXPECT_EQ(item->getValue(),    2);
        EXPECT_EQ(item->getRarity(),   "Common");
        EXPECT_FALSE(item->getRequiresAttunement());

        // Weapon-specific fields preserved
        Weapon* w = dynamic_cast<Weapon*>(item.get());
        ASSERT_NE(w, nullptr);
        EXPECT_EQ(w->getDamageDice(),     "1d4");
        EXPECT_EQ(w->getDamageType(),     "Piercing");
        EXPECT_EQ(w->getWeaponCategory(), "Simple");
        EXPECT_EQ(w->getWeaponType(),     "Melee");
        EXPECT_EQ(w->getProperties(),     "Finesse, Light, Thrown");
        EXPECT_EQ(w->getRange(),          "20/60 ft");
    }
}

// ── Armor save/load roundtrip ──────────────────────────────────────────────────

TEST(PersistenceTest, ArmorRoundtrip) {
    TempFile tmp("dnd_test_armor.tmp");

    {
        Armor a("Chain Mail", "Interlocking metal rings", 55.0f, 1, 75, "Common", false,
                "Heavy", /*baseAC*/ 16, /*maxDexBonus*/ 0, /*strReq*/ 13, /*stealthDisadv*/ true);
        std::ofstream out(tmp.path);
        ASSERT_TRUE(out.is_open());
        a.save(out);
    }

    {
        std::ifstream in(tmp.path);
        ASSERT_TRUE(in.is_open());
        auto item = Item::loadFromFile(in);
        ASSERT_NE(item, nullptr);

        EXPECT_EQ(item->getName(), "Chain Mail");

        Armor* a = dynamic_cast<Armor*>(item.get());
        ASSERT_NE(a, nullptr);
        EXPECT_EQ(a->getArmorType(),      "Heavy");
        EXPECT_EQ(a->getBaseAC(),         16);
        EXPECT_EQ(a->getMaxDexBonus(),    0);
        EXPECT_EQ(a->getStrRequirement(), 13);
        EXPECT_TRUE(a->getStealthDisadvantage());
    }
}

TEST(PersistenceTest, ArmorNoDexCapRoundtrip) {
    TempFile tmp("dnd_test_armor2.tmp");

    {
        Armor a("Leather Armor", "Light armor", 10.0f, 1, 10, "Common", false,
                "Light", 11, /*maxDexBonus*/ -1, 0, false);
        std::ofstream out(tmp.path);
        a.save(out);
    }

    {
        std::ifstream in(tmp.path);
        auto item = Item::loadFromFile(in);
        Armor* a = dynamic_cast<Armor*>(item.get());
        ASSERT_NE(a, nullptr);
        EXPECT_EQ(a->getMaxDexBonus(), -1);    // -1 = no cap must survive roundtrip
        EXPECT_FALSE(a->getStealthDisadvantage());
    }
}

// ── Gear save/load roundtrip ───────────────────────────────────────────────────

TEST(PersistenceTest, GearRoundtrip) {
    TempFile tmp("dnd_test_gear.tmp");

    {
        Gear g("Torch", "A hand-held light source", 1.0f, 10, 0, "Common", false);
        std::ofstream out(tmp.path);
        ASSERT_TRUE(out.is_open());
        g.save(out);
    }

    {
        std::ifstream in(tmp.path);
        ASSERT_TRUE(in.is_open());
        auto item = Item::loadFromFile(in);
        ASSERT_NE(item, nullptr);

        EXPECT_EQ(item->getName(),     "Torch");
        EXPECT_EQ(item->getQuantity(), 10);
        EXPECT_FLOAT_EQ(item->getWeight(), 1.0f);
    }
}

// ── Item::loadFromFile factory pattern ────────────────────────────────────────

TEST(PersistenceTest, FactoryReconstructsWeaponSubclass) {
    TempFile tmp("dnd_test_factory_w.tmp");
    Weapon w("Sword", "", 3.0f, 1, 15, "Common", false,
             "1d8", "Slashing", "Martial", "Melee", "Versatile", "5 ft");
    { std::ofstream out(tmp.path); w.save(out); }

    std::ifstream in(tmp.path);
    auto item = Item::loadFromFile(in);
    EXPECT_NE(dynamic_cast<Weapon*>(item.get()), nullptr);
    EXPECT_EQ(item->getItemTag(), "WEAPON");
}

TEST(PersistenceTest, FactoryReconstructsArmorSubclass) {
    TempFile tmp("dnd_test_factory_a.tmp");
    Armor a("Shield", "", 6.0f, 1, 10, "Common", false, "Shield", 2, -1, 0, false);
    { std::ofstream out(tmp.path); a.save(out); }

    std::ifstream in(tmp.path);
    auto item = Item::loadFromFile(in);
    EXPECT_NE(dynamic_cast<Armor*>(item.get()), nullptr);
    EXPECT_EQ(item->getItemTag(), "ARMOR");
}

TEST(PersistenceTest, FactoryReconstructsGearSubclass) {
    TempFile tmp("dnd_test_factory_g.tmp");
    Gear g("Rope", "", 10.0f, 1, 1, "Common", false);
    { std::ofstream out(tmp.path); g.save(out); }

    std::ifstream in(tmp.path);
    auto item = Item::loadFromFile(in);
    EXPECT_NE(dynamic_cast<Gear*>(item.get()), nullptr);
    EXPECT_EQ(item->getItemTag(), "GEAR");
}

// ── SpellBook save/load roundtrip ──────────────────────────────────────────────

TEST(PersistenceTest, SpellBookRoundtrip) {
    TempFile tmp("dnd_test_spellbook.tmp");

    {
        SpellBook sb;
        sb.addSpell(Spell("Fireball",     "Evocation",   "Damage",  3,
                          "1 action", "150 ft",  "V, S, M", "Instantaneous", "Dexterity",
                          "Fiery explosion at a point."));
        sb.addSpell(Spell("Magic Missile","Evocation",   "Damage",  1,
                          "1 action", "120 ft",  "V, S",    "Instantaneous", "None",
                          "Unerring bolts of force."));
        sb.saveSpellBook(tmp.path);
    }

    {
        SpellBook sb;
        sb.loadSpellBook(tmp.path);
        auto spells = sb.getAllSpells();
        ASSERT_EQ(spells.size(), 2u);
        EXPECT_EQ(spells[0].getSpellName(),  "Fireball");
        EXPECT_EQ(spells[0].getSpellLevel(), 3);
        EXPECT_EQ(spells[1].getSpellName(),  "Magic Missile");
        EXPECT_EQ(spells[1].getSpellLevel(), 1);
    }
}

TEST(PersistenceTest, SpellBookRoundtripPreservesAllFields) {
    TempFile tmp("dnd_test_spellbook2.tmp");

    Spell original("Shield", "Abjuration", "Defense", 1,
                   "1 reaction", "Self", "V, S", "1 round", "None",
                   "An invisible barrier of magical force appears.");
    {
        SpellBook sb;
        sb.addSpell(original);
        sb.saveSpellBook(tmp.path);
    }

    {
        SpellBook sb;
        sb.loadSpellBook(tmp.path);
        auto spells = sb.getAllSpells();
        ASSERT_EQ(spells.size(), 1u);
        const Spell& s = spells[0];
        EXPECT_EQ(s.getSpellName(),        "Shield");
        EXPECT_EQ(s.getSpellType(),        "Abjuration");
        EXPECT_EQ(s.getSpellEffect(),      "Defense");
        EXPECT_EQ(s.getSpellLevel(),       1);
        EXPECT_EQ(s.getSpellTime(),        "1 reaction");
        EXPECT_EQ(s.getSpellRange(),       "Self");
        EXPECT_EQ(s.getSpellComponents(),  "V, S");
        EXPECT_EQ(s.getSpellDuration(),    "1 round");
        EXPECT_EQ(s.getSpellSavingThrow(), "None");
        EXPECT_EQ(s.getSpellDescription(), "An invisible barrier of magical force appears.");
    }
}

TEST(PersistenceTest, SpellBookLoadClearsExistingSpells) {
    TempFile tmp("dnd_test_spellbook3.tmp");

    // Save a book with one spell
    {
        SpellBook sb;
        sb.addSpell(Spell("Fireball", "Evocation", "Damage", 3,
                          "1 action", "150 ft", "V, S, M", "Instantaneous", "Dexterity", "Explosion."));
        sb.saveSpellBook(tmp.path);
    }

    // Load into a spellbook that already has a different spell
    SpellBook sb;
    sb.addSpell(Spell("Magic Missile", "Evocation", "Damage", 1,
                      "1 action", "120 ft", "V, S", "Instantaneous", "None", "Bolts."));
    sb.loadSpellBook(tmp.path);

    auto spells = sb.getAllSpells();
    ASSERT_EQ(spells.size(), 1u);         // old spell replaced, not appended
    EXPECT_EQ(spells[0].getSpellName(), "Fireball");
}

TEST(PersistenceTest, CharacterDirectoryRoundtripPreservesStatusTracking) {
    namespace fs = std::filesystem;
    const fs::path dir = "dnd_test_character_dir";
    fs::remove_all(dir);
    fs::create_directories(dir);

    Character original("Cleric", "Human", "Cleric", "Acolyte", "Lawful Good",
                       4, 32, 170, 0, 28, 0, "d8",
                       10, 12, 14, 10, 16, 12, 1, 2);
    original.setDeathSaveSuccesses(2);
    original.setDeathSaveFailures(1);
    original.addCondition("Poisoned");
    original.addCondition("Prone");
    original.getFeatures().setSkillRank("Perception", SkillRank::Proficient);
    original.saveToDirectory(dir.string());

    Character loaded = Character::loadFromDirectory(dir.string());

    EXPECT_EQ(loaded.getDeathSaveSuccesses(), 2);
    EXPECT_EQ(loaded.getDeathSaveFailures(), 1);
    ASSERT_EQ(loaded.getConditions().size(), 2u);
    EXPECT_EQ(loaded.getConditions()[0], "Poisoned");
    EXPECT_EQ(loaded.getConditions()[1], "Prone");
    EXPECT_EQ(loaded.getPassivePerception(), 15);

    fs::remove_all(dir);
}
