#ifndef CHARACTER_H
#define CHARACTER_H

#include <string>
#include <fstream>
#include <limits>
#include <cmath>
#include <vector>
#include <memory>
#include "H_Inventory.h"
#include "H_SpellBook.h"
#include "H_SpellSlots.h"
#include "H_Wallet.h"
#include "H_CharacterFeatures.h"

enum class DeathSaveOutcome {
    None = 0,
    Stable = 1,
    Dead = 2,
    Revived = 3
};

class Character {
private:

    // Character details
    std::string name;
    std::string race;
    std::string characterClass;
    std::string background;
    std::string alignment;
    int level;
    int age;
    int weight;

    // Health
    int current_hp;
    int max_hp;
    int temp_hp;
    int deathSaveSuccesses;
    int deathSaveFailures;
    std::vector<std::string> conditions;

    //Hit die
    int hit_die_num;
    std::string hit_dice;

    // Stats
    int strength;
    int dexterity;
    int constitution;
    int intelligence;
    int wisdom;
    int charisma;
    int Initiative;
    int proficiency;


    Inventory inventory;
    Spellbook spellbook;
    SpellSlots spellSlots;
    Wallet wallet;
    CharacterFeatures features;

    int equippedArmorIndex;   // 1-based inventory index, -1 = none
    int equippedShieldIndex;  // 1-based inventory index, -1 = none

    bool inspiration;
    int speed;

public:
    Character(std::string n, std::string r, std::string c, std::string b, std::string a,
         int lvl, int age, int weight,int c_hp, int m_hp, int t_hp, std::string h_dice, int str, int dex, int con, int intl, int wis, int cha, int init, int prof);

    // Display functions
    void display() const;

    // Getters
    std::string getName() const;
    std::string getRace() const;
    std::string getClass() const;
    std::string getBackground() const;
    std::string getAlignment() const;
    int getLevel() const;
    int getAge() const;
    int getWeight() const;

    int getCurrentHP() const;
    int getMaxHP() const;
    int getTempHP() const;
    int getDeathSaveSuccesses() const;
    int getDeathSaveFailures() const;
    const std::vector<std::string>& getConditions() const;
    std::string getHitDice() const;
    int getHitDiceNum() const;

    void setHitDiceNum(int n);


    int getStrength() const;
    int getDexterity() const;
    int getConstitution() const;
    int getIntelligence() const;
    int getWisdom() const;
    int getCharisma() const;
    int getInitiative() const;
    int getProficiency() const;

    int getAbilityModifier(int& ability_score);

    // Setters
    void setName(const std::string& n);
    void setRace(const std::string& r);
    void setClass(const std::string& c);
    void setBackground(const std::string& b);
    void setAlignment(const std::string& a);
    void setLevel(int lvl);
    void setAge(int age);
    void setWeight(int weight);

    void setCurrentHP(int c_hp);
    void setMaxHP(int m_hp);
    void setTempHP(int t_hp);
    void setDeathSaveSuccesses(int successes);
    void setDeathSaveFailures(int failures);
    void resetDeathSaves();
    DeathSaveOutcome applyDeathSaveRoll(int roll);
    void addCondition(const std::string& condition);
    bool removeCondition(int index);
    void clearConditions();
    void setHitDice(const std::string& new_hit_dice);
    void spendHitDice(int count);
    void recoverHitDice();

    void setStrength(int str);
    void setDexterity(int dex);
    void setConstitution(int con);
    void setIntelligence(int intl);
    void setWisdom(int wis);
    void setCharisma(int cha);
    void setInitiative(int init);
    void setProficiency(int prof);

    bool getInspiration() const;
    void setInspiration(bool value);
    void toggleInspiration();

    int getSpeed() const;
    void setSpeed(int spd);
    int getPassivePerception() const;

    void setStats(int new_AS, int ability_to_change);

    // Inventory
    void addItem(std::unique_ptr<Item> item);
    void removeItem(int index);
    void showInventory() const;
    void clearInventory();
    Inventory& getInventory();
    const Inventory& getInventory() const;

    // AC
    int getAC() const;
    void equipArmor(int index);
    void equipShield(int index);
    void unequipArmor();
    void unequipShield();
    int getEquippedArmorIndex() const;
    int getEquippedShieldIndex() const;
    void setEquippedArmorIndex(int idx);
    void setEquippedShieldIndex(int idx);

    // Spellbook
    Spellbook& getSpellbook();
    const Spellbook& getSpellbook() const;
    SpellSlots& getSpellSlots();
    const SpellSlots& getSpellSlots() const;
    void showSpells() const;

    // Wallet
    Wallet& getWallet();
    const Wallet& getWallet() const;
    void showCurrency() const;

    // Features and skills
    CharacterFeatures& getFeatures();
    const CharacterFeatures& getFeatures() const;
    void showFeatures() const;

    // File functions
    void saveToDirectory(const std::string& dirPath) const;
    static Character loadFromDirectory(const std::string& dirPath);
};

#endif
