#include "H_Character.h"
#include "H_Weapon.h"
#include "H_Armor.h"
#include "H_Gear.h"
#include "H_DndExceptions.h"
#include "H_Validate.h"
#include "H_SaveFormat.h"
#include <iostream>
#include <filesystem>
#include <sstream>
#include <vector>

// Constructor
Character::Character(std::string n, std::string r, std::string c, std::string b, std::string a,
     int lvl, int new_age, int new_weight, int c_hp, int m_hp, int t_hp, std::string h_dice, int str, int dex, int con, int intl, int wis, int cha, int init, int prof) {
    name = n;
    race = r;
    characterClass = c;
    background = b;
    alignment = a;
    level = lvl;
    hitDiceNum = lvl;
    age = new_age;
    weight = new_weight;
    currentHp = c_hp;
    maxHp = m_hp;
    tempHp = t_hp;
    deathSaveSuccesses = 0;
    deathSaveFailures = 0;
    hitDice = h_dice;
    strength = str;
    dexterity = dex;
    constitution = con;
    intelligence = intl;
    wisdom = wis;
    charisma = cha;
    initiative = init;
    proficiency = prof;
    equippedArmorIndex = -1;
    equippedShieldIndex = -1;
    inspiration = false;
    speed = 30;
}



// Save all character data into a dedicated directory, one file per section.
void Character::saveToDirectory(const std::string& dir) const {
    namespace fs = std::filesystem;

    {
        std::ofstream f((fs::path(dir) / "character.txt").string());
        if (!f) throw SaveError("cannot open character.txt in " + dir);
        SaveFormat::writeHeader(f, SaveFormat::kCharacterTag);
        f << name << "\n" << race << "\n" << characterClass << "\n"
          << background << "\n" << alignment << "\n"
          << level << " " << age << " " << weight << "\n"
          << currentHp << " " << maxHp << " " << tempHp << "\n"
          << deathSaveSuccesses << " " << deathSaveFailures << "\n"
          << hitDice << " " << hitDiceNum << "\n"
          << strength << " " << dexterity << " " << constitution << " "
          << intelligence << " " << wisdom << " " << charisma << " "
          << initiative << " " << proficiency << "\n"
          << equippedArmorIndex << " " << equippedShieldIndex << "\n"
          << (inspiration ? 1 : 0) << "\n"
          << speed << "\n"
          << conditions.size() << "\n";
        for (const auto& condition : conditions)
        {
            f << condition << "\n";
        }
    }
    {
        std::ofstream f((fs::path(dir) / "inventory.txt").string());
        if (!f) throw SaveError("cannot open inventory.txt in " + dir);
        inventory.save(f);
    }
    {
        std::ofstream f((fs::path(dir) / "wallet.txt").string());
        if (!f) throw SaveError("cannot open wallet.txt in " + dir);
        wallet.save(f);
    }
    spellbook.saveSpellBook((fs::path(dir) / "spells.txt").string());
    {
        std::ofstream f((fs::path(dir) / "spellslots.txt").string());
        if (!f) throw SaveError("cannot open spellslots.txt in " + dir);
        spellSlots.save(f);
    }
    {
        std::ofstream f((fs::path(dir) / "features.txt").string());
        if (!f) throw SaveError("cannot open features.txt in " + dir);
        features.save(f);
    }
}

// Reconstruct a Character from a previously saved directory.

Character Character::loadFromDirectory(const std::string& dir,
                                       std::vector<std::string>* repairs)
{
    namespace fs = std::filesystem;

    // Read the whole file up front so fields can be validated per line rather
    // than streamed into variables that keep stale values when a read fails.
    std::vector<std::string> lines;
    {
        std::ifstream f((fs::path(dir) / "character.txt").string());
        if (!f) throw LoadError("cannot open character.txt in " + dir);
        std::string line;
        while (std::getline(f, line))
        {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            lines.push_back(line);
        }
    }

    // Detect the format version. Files written before versioning have no
    // header and begin directly with the character name.
    const std::string what = dir + "/character.txt";
    int version = 0;
    size_t at = 0;
    {
        std::istringstream head(lines.empty() ? std::string() : lines[0] + "\n");
        version = SaveFormat::readHeader(head, SaveFormat::kCharacterTag, what,
                                         kSaveFormatVersion);
        if (version > 0) at = 1;
    }
    // An older-but-valid file is not damaged -- it is simply written in an
    // earlier layout and is upgraded the next time it is saved. Only genuine
    // field problems are reported as repairs.

    // Both v0 and v1 share the same field layout; v1 only adds the header.
    // A future v2 would branch here rather than reinterpreting v1 positions.
    if (lines.size() < at + 9)
    {
        throw LoadError("character.txt in " + dir + " is truncated");
    }

    auto nextLine = [&]() -> const std::string& { return lines[at++]; };

    const std::string name           = nextLine();
    const std::string race           = nextLine();
    const std::string characterClass = nextLine();
    const std::string background     = nextLine();
    const std::string alignment      = nextLine();

    if (!Validate::isName(name)) throw LoadError("character.txt in " + dir + " has no name");

    std::vector<int> f;

    if (!SaveFormat::parseInts(nextLine(), 3, f)) throw LoadError("bad level/age/weight line in " + dir);
    const int lvl    = SaveFormat::repairRange(f[0], 1, 20, 1, "level", repairs);
    const int age    = SaveFormat::repairRange(f[1], 0, 100000, 0, "age", repairs);
    const int weight = SaveFormat::repairRange(f[2], 0, 100000, 0, "weight", repairs);

    if (!SaveFormat::parseInts(nextLine(), 3, f)) throw LoadError("bad hp line in " + dir);
    const int m_hp = SaveFormat::repairRange(f[1], 1, 100000, 1, "max hp", repairs);
    const int c_hp = SaveFormat::repairRange(f[0], 0, m_hp, m_hp, "current hp", repairs);
    const int t_hp = SaveFormat::repairRange(f[2], 0, 100000, 0, "temp hp", repairs);

    if (!SaveFormat::parseInts(nextLine(), 2, f)) throw LoadError("bad death save line in " + dir);
    const int deathSuccesses = SaveFormat::repairRange(f[0], 0, 3, 0, "death save successes", repairs);
    const int deathFailures  = SaveFormat::repairRange(f[1], 0, 3, 0, "death save failures", repairs);

    // Hit dice: the field that silently held "0" in saves written by older builds.
    std::string h_dice;
    int h_dice_num = lvl;
    {
        std::istringstream ss(nextLine());
        std::string numTok;
        ss >> h_dice >> numTok;
        if (!Validate::isHitDice(h_dice))
        {
            if (repairs)
            {
                repairs->push_back("hit dice was \"" + h_dice +
                                   "\", not a valid dN value; set to d8");
            }
            h_dice = "d8";
        }
        if (!SaveFormat::parseWholeInt(numTok, h_dice_num)) h_dice_num = lvl;
        h_dice_num = SaveFormat::repairRange(h_dice_num, 0, lvl, lvl, "hit dice remaining", repairs);
    }

    if (!SaveFormat::parseInts(nextLine(), 8, f)) throw LoadError("bad ability score line in " + dir);
    const int str  = SaveFormat::repairRange(f[0], 1, 30, 10, "strength", repairs);
    const int dex  = SaveFormat::repairRange(f[1], 1, 30, 10, "dexterity", repairs);
    const int con  = SaveFormat::repairRange(f[2], 1, 30, 10, "constitution", repairs);
    const int intl = SaveFormat::repairRange(f[3], 1, 30, 10, "intelligence", repairs);
    const int wis  = SaveFormat::repairRange(f[4], 1, 30, 10, "wisdom", repairs);
    const int cha  = SaveFormat::repairRange(f[5], 1, 30, 10, "charisma", repairs);
    const int init = SaveFormat::repairRange(f[6], -10, 20, 0, "initiative", repairs);
    // Proficiency is +2 at level 1 and never lower; +6 is the level-20 cap.
    const int prof = SaveFormat::repairRange(f[7], 2, 6, 2, "proficiency", repairs);

    int armorIdx = -1, shieldIdx = -1, insp = 0, spd = 30;
    if (at < lines.size() && SaveFormat::parseInts(lines[at], 2, f))
    {
        armorIdx = SaveFormat::repairRange(f[0], -1, 100000, -1, "equipped armor index", repairs);
        shieldIdx = SaveFormat::repairRange(f[1], -1, 100000, -1, "equipped shield index", repairs);
        at++;
    }
    if (at < lines.size() && SaveFormat::parseWholeInt(lines[at], insp)) at++;
    if (at < lines.size() && SaveFormat::parseWholeInt(lines[at], spd)) at++;
    spd = SaveFormat::repairRange(spd, 0, 1000, 30, "speed", repairs);

    std::vector<std::string> loadedConditions;
    int conditionCount = 0;
    if (at < lines.size() && SaveFormat::parseWholeInt(lines[at], conditionCount))
    {
        at++;
        for (int i = 0; i < conditionCount && at < lines.size(); i++)
        {
            const std::string condition = lines[at++];
            if (!condition.empty()) loadedConditions.push_back(condition);
        }
    }

    Character c(name, race, characterClass, background, alignment,
                lvl, age, weight, c_hp, m_hp, t_hp, h_dice,
                str, dex, con, intl, wis, cha, init, prof);
    c.setHitDiceNum(h_dice_num);
    c.setEquippedArmorIndex(armorIdx);
    c.setEquippedShieldIndex(shieldIdx);
    c.setInspiration(insp != 0);
    c.setSpeed(spd);
    c.setDeathSaveSuccesses(deathSuccesses);
    c.setDeathSaveFailures(deathFailures);
    for (const auto& condition : loadedConditions)
    {
        c.addCondition(condition);
    }

    {
        std::ifstream f2((fs::path(dir) / "inventory.txt").string());
        if (f2) c.getInventory().load(f2);
    }
    {
        std::ifstream f2((fs::path(dir) / "wallet.txt").string());
        if (f2) c.getWallet().load(f2);
    }
    // Guarded like the other sub-files: loadSpellBook throws if the file is
    // absent, which would otherwise make a missing spells.txt fail the whole
    // character load while a missing inventory.txt is tolerated.
    if (fs::exists(fs::path(dir) / "spells.txt"))
    {
        c.getSpellBook().loadSpellBook((fs::path(dir) / "spells.txt").string());
    }
    {
        std::ifstream f2((fs::path(dir) / "spellslots.txt").string());
        if (f2) c.getSpellSlots().load(f2);
    }
    {
        std::ifstream f2((fs::path(dir) / "features.txt").string());
        if (f2) c.getFeatures().load(f2);
    }

    return c;
}

// Display
void Character::display() const {



    auto mod = [](int score) -> std::string {
        int m = (score / 2) - 5;
        return (m >= 0 ? "+" : "") + std::to_string(m);
    };

    std::cout << "\n=== Character Sheet ===\n";
    std::cout << "Name: "       << name           << "\n";
    std::cout << "Race: "       << race           << "\n";
    std::cout << "Class: "      << characterClass << "\n";
    std::cout << "Level: "      << level          << "\n";
    std::cout << "Age: "        << age            << "\n";
    std::cout << "Background: " << background     << "\n";
    std::cout << "Alignment: "  << alignment      << "\n";
    std::cout << "Weight: "     << weight         << " lbs\n";
    std::cout << "HP: "         << currentHp << "/" << maxHp;
    if (tempHp > 0) std::cout << "  (+" << tempHp << " temp)";
    std::cout << "\n";
    std::cout << "Death Saves: " << deathSaveSuccesses << " success, "
              << deathSaveFailures << " failure\n";
    std::cout << "AC: "         << getAC();
    if (equippedArmorIndex > 0 && equippedArmorIndex <= inventory.size())
        std::cout << "  [" << inventory.getItem(equippedArmorIndex).getName() << "]";
    if (equippedShieldIndex > 0 && equippedShieldIndex <= inventory.size())
        std::cout << " + Shield";
    std::cout << "\n";
    std::cout << "Hit Dice: "   << hitDiceNum << "/" << level << hitDice << "\n";
    std::cout << "STR: " << strength     << " (" << mod(strength)     << ")\n";
    std::cout << "DEX: " << dexterity    << " (" << mod(dexterity)    << ")\n";
    std::cout << "CON: " << constitution << " (" << mod(constitution) << ")\n";
    std::cout << "INT: " << intelligence << " (" << mod(intelligence) << ")\n";
    std::cout << "WIS: " << wisdom       << " (" << mod(wisdom)       << ")\n";
    std::cout << "CHA: " << charisma     << " (" << mod(charisma)     << ")\n";
    std::cout << "initiative: +" << initiative << "\n";
    std::cout << "Proficiency: +" << proficiency << "\n";
    std::cout << "Passive Perception: " << getPassivePerception() << "\n";
    std::cout << "Speed: " << speed << " ft\n";
    std::cout << "Inspiration: " << (inspiration ? "Yes" : "No") << "\n";
    std::cout << "Conditions: ";
    if (conditions.empty())
    {
        std::cout << "None\n";
    }
    else
    {
        for (size_t i = 0; i < conditions.size(); i++)
        {
            std::cout << conditions[i];
            if (i + 1 < conditions.size())
            {
                std::cout << ", ";
            }
        }
        std::cout << "\n";
    }
    std::cout << "Feats: " << features.getFeats().size() << "\n";
    std::cout << "Racial Traits: " << features.getRacialTraits().size() << "\n";
    std::cout << "Languages: " << features.getLanguages().size() << "\n";
    std::cout << "Items: " << inventory.size() << " carried\n";
    std::cout << "Currency: ";
    wallet.display();
    std::cout << "=======================\n";
}


// Getters

std::string Character::getName() const {return name;}
std::string Character::getRace() const {return race;}
std::string Character::getClass() const {return characterClass;}
std::string Character::getBackground() const{return background;}
std::string Character::getAlignment() const{return alignment;}
int Character::getLevel() const {return level;}
int Character::getAge() const{return age;}
int Character::getWeight() const{return weight;}
int Character::getCurrentHP() const{return currentHp;}
int Character::getMaxHP() const{return maxHp;}
int Character::getTempHP() const{return tempHp;}
int Character::getDeathSaveSuccesses() const { return deathSaveSuccesses; }
int Character::getDeathSaveFailures() const { return deathSaveFailures; }
const std::vector<std::string>& Character::getConditions() const { return conditions; }
std::string Character::getHitDice() const{return hitDice;}
int Character::getStrength() const {return strength;}
int Character::getDexterity() const {return dexterity;}
int Character::getConstitution() const {return constitution;}
int Character::getIntelligence() const {return intelligence;}
int Character::getWisdom() const {return wisdom;}
int Character::getCharisma() const {return charisma;}
int Character::getInitiative() const {return initiative;}
int Character::getProficiency() const {return proficiency;}
// D&D modifier is floor((score - 10) / 2). Ability scores are always positive,
// so integer division truncates toward zero and matches floor here.
int Character::getAbilityModifier(int ability_score) { return (ability_score / 2) - 5; }

// Setters

void Character::setName(const std::string& n) { name = n; }
void Character::setRace(const std::string& r) { race = r; }
void Character::setClass(const std::string& c) { characterClass = c; }
void Character::setBackground(const std::string& b) { background = b;}
void Character::setAlignment(const std::string& a) {alignment = a;}
void Character::setLevel(int lvl){level = lvl;}
void Character::setAge(int a){age = a;}
void Character::setWeight(int w){weight = w;}
void Character::setCurrentHP(int c_hp)
{
    currentHp = c_hp;
    // Any healing above 0 HP clears the death save track.
    if (currentHp > 0)
    {
        resetDeathSaves();
    }
}
void Character::setMaxHP(int m_hp){maxHp = m_hp;}
void Character::setTempHP(int t_hp){tempHp = t_hp;}
void Character::setDeathSaveSuccesses(int successes)
{
    deathSaveSuccesses = (successes < 0 ? 0 : (successes > 3 ? 3 : successes));
}
void Character::setDeathSaveFailures(int failures)
{
    deathSaveFailures = (failures < 0 ? 0 : (failures > 3 ? 3 : failures));
}
void Character::resetDeathSaves()
{
    // Death save state is tracked separately from HP, so reset both counters together.
    deathSaveSuccesses = 0;
    deathSaveFailures = 0;
}
DeathSaveOutcome Character::applyDeathSaveRoll(int roll)
{
    // Natural 1 counts as two failed saves.
    if (roll <= 1)
    {
        setDeathSaveFailures(deathSaveFailures + 2);
        if (deathSaveFailures >= 3)
        {
            resetDeathSaves();
            return DeathSaveOutcome::Dead;
        }
        return DeathSaveOutcome::None;
    }

    // Natural 20 immediately brings the character back to 1 HP.
    if (roll == 20)
    {
        setCurrentHP(1);
        return DeathSaveOutcome::Revived;
    }

    // 10 or higher is a success; three total successes stabilizes the character.
    if (roll >= 10)
    {
        setDeathSaveSuccesses(deathSaveSuccesses + 1);
        if (deathSaveSuccesses >= 3)
        {
            resetDeathSaves();
            return DeathSaveOutcome::Stable;
        }
    }
    else
    {
        // 9 or lower is a failed save; three total failures means death.
        setDeathSaveFailures(deathSaveFailures + 1);
        if (deathSaveFailures >= 3)
        {
            resetDeathSaves();
            return DeathSaveOutcome::Dead;
        }
    }

    return DeathSaveOutcome::None;
}
void Character::addCondition(const std::string& condition)
{
    // Conditions are stored as free-form labels like "Poisoned" or "Stunned".
    if (!condition.empty())
    {
        conditions.push_back(condition);
    }
}
bool Character::removeCondition(int index)
{
    // The UI shows conditions starting at 1, so convert that to the vector's 0-based index.
    if (index < 1 || index > static_cast<int>(conditions.size()))
    {
        return false;
    }

    conditions.erase(conditions.begin() + index - 1);
    return true;
}
void Character::clearConditions()
{
    // Removes every active condition from the character at once.
    conditions.clear();
}
void Character::setHitDice(const std::string& new_hit_dice){hitDice = new_hit_dice;}
int Character::getHitDiceNum() const { return hitDiceNum; }
void Character::setHitDiceNum(int n) { hitDiceNum = (n < 0 ? 0 : (n > level ? level : n)); }


// Hit die utilities
void Character::spendHitDice(int count)
{
    if (count < 1 || count > hitDiceNum) return;
    hitDiceNum -= count;
}

void Character::recoverHitDice()
{
    // On a long rest, recover half the character's total hit dice (rounded up).
    int recovered = (level + 1) / 2;
    hitDiceNum = (hitDiceNum + recovered > level) ? level : hitDiceNum + recovered;
}

// Setters Cont.
void Character::setStrength(int str) { strength = str; }
void Character::setDexterity(int dex) {dexterity = dex;}
void Character::setConstitution(int con) {constitution = con;}
void Character::setIntelligence(int intl) {intelligence = intl;}
void Character::setWisdom(int wis) {wisdom = wis;}
void Character::setCharisma(int cha) {charisma = cha;}
void Character::setInitiative(int init) {initiative = init;}
void Character::setProficiency(int prof) {proficiency = prof;}

bool Character::getInspiration() const { return inspiration; }
void Character::setInspiration(bool value) { inspiration = value; }
void Character::toggleInspiration() { inspiration = !inspiration; }

int Character::getSpeed() const { return speed; }
void Character::setSpeed(int spd) { if (spd > 0) speed = spd; }
int Character::getPassivePerception() const
{
    return 10 + features.getSkillModifier("Perception",
                                          strength, dexterity, constitution,
                                          intelligence, wisdom, charisma,
                                          proficiency);
}



// Inventory
void Character::addItem(std::unique_ptr<Item> item) {
    inventory.addItem(std::move(item));
}

Inventory& Character::getInventory() { return inventory; }
const Inventory& Character::getInventory() const { return inventory; }

void Character::removeItem(int index) {
    if (index == equippedArmorIndex)       equippedArmorIndex = -1;
    else if (index < equippedArmorIndex)   equippedArmorIndex--;

    if (index == equippedShieldIndex)      equippedShieldIndex = -1;
    else if (index < equippedShieldIndex)  equippedShieldIndex--;

    inventory.removeItem(index);
}

void Character::showInventory() const {
    inventory.display();
}

void Character::clearInventory() {
    while (inventory.size() > 0) {
        inventory.removeItem(1);
    }
}

// AC and Armor management
int Character::getAC() const {
    int dexMod = (dexterity / 2) - 5;
    int ac = 10 + dexMod; // unarmored default

    // Calculates ac based on armour type ie light, medium, heavy, and whether a shield is equiped
    if (equippedArmorIndex > 0 && equippedArmorIndex <= inventory.size()) {
        const Armor* armor = dynamic_cast<const Armor*>(&inventory.getItem(equippedArmorIndex));
        if (armor) {
            const std::string type = armor->getArmorType();
            int base = armor->getBaseAC();
            int maxDex = armor->getMaxDexBonus();

            if (type == "Light") {
                ac = base + dexMod;
            } else if (type == "Medium") {
                int cap = (maxDex >= 0) ? std::min(dexMod, maxDex) : dexMod;
                ac = base + cap;
            } else {
                ac = base; // Heavy: no DEX bonus
            }
        }
    }

    if (equippedShieldIndex > 0 && equippedShieldIndex <= inventory.size())
        ac += 2;

    return ac;
}

void Character::equipArmor(int index) {
    if (index < 1 || index > inventory.size()) return;
    const Armor* armor = dynamic_cast<const Armor*>(&inventory.getItem(index));
    if (armor && armor->getArmorType() != "Shield")
        equippedArmorIndex = index;
}

void Character::equipShield(int index) {
    if (index < 1 || index > inventory.size()) return;
    const Armor* armor = dynamic_cast<const Armor*>(&inventory.getItem(index));
    if (armor && armor->getArmorType() == "Shield")
        equippedShieldIndex = index;
}

void Character::unequipArmor()  { equippedArmorIndex  = -1; }
void Character::unequipShield() { equippedShieldIndex = -1; }

int  Character::getEquippedArmorIndex()  const { return equippedArmorIndex;  }
int  Character::getEquippedShieldIndex() const { return equippedShieldIndex; }
void Character::setEquippedArmorIndex(int idx)  { equippedArmorIndex  = idx; }
void Character::setEquippedShieldIndex(int idx) { equippedShieldIndex = idx; }

// Ability scores
void Character::setStats(int AS, int Ability)
{
    if(AS <= 20 && AS > 0)
    {
        switch (Ability)
        {
        case 1:
            strength = AS;
            break;

        case 2:
            dexterity = AS;
            break;

        case 3:
            constitution = AS;
            break;

        case 4:
            intelligence = AS;
            break;

        case 5:
            wisdom = AS;
            break;

        case 6:
            charisma = AS;
            break;

        default:
            std::cout << "Not an ability";
        }
    }
    else
    {
        // Callers validate the range before calling; this stays as a guard so a
        // bad value is rejected rather than silently stored. Console recovery is
        // the caller's job -- a model class should not be touching std::cin.
        std::cout << "Ability score must be between 1 and 20" << std::endl;
    }
}

//-----------------------------------------------------//
// Wallet
Wallet& Character::getWallet() { return wallet; }
const Wallet& Character::getWallet() const { return wallet; }

void Character::showCurrency() const {
    std::cout << "\n=== Currency ===\n";
    wallet.display();
}

CharacterFeatures& Character::getFeatures()
{
    return features;
}

const CharacterFeatures& Character::getFeatures() const
{
    return features;
}

void Character::showFeatures() const
{
    // Present all tracked feature-style character data together in one place.
    std::cout << "\n=== Feats ===\n";
    features.displayFeats();

    std::cout << "\n=== Racial Traits ===\n";
    features.displayRacialTraits();

    std::cout << "\n=== Saving Throws ===\n";
    features.displaySaves(strength, dexterity, constitution,
                          intelligence, wisdom, charisma,
                          proficiency);

    std::cout << "\n=== Skills ===\n";
    features.displaySkills(strength, dexterity, constitution,
                           intelligence, wisdom, charisma,
                           proficiency);

    std::cout << "\n=== Languages ===\n";
    features.displayLanguages();

    std::cout << "Inspiration: " << (inspiration ? "Yes" : "No") << "\n";
}

//-----------------------------------------------------//
// SpellBook
SpellBook& Character::getSpellBook()
{
    return spellbook;
}

const SpellBook& Character::getSpellBook() const
{
    return spellbook;
}

SpellSlots& Character::getSpellSlots()
{
    return spellSlots;
}

const SpellSlots& Character::getSpellSlots() const
{
    return spellSlots;
}

void Character::showSpells() const
{
    std::cout << "\n=== SpellBook ===\n";
    spellbook.displayAllSpells();

    std::cout << "\n=== Spell Slots ===\n";
    spellSlots.displaySlots();
}
