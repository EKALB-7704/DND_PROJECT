#include "H_Character.h"
#include "H_Weapon.h"
#include "H_Armor.h"
#include "H_Gear.h"
#include "H_DndExceptions.h"
#include <iostream>
#include <filesystem>

// Constructor
Character::Character(std::string n, std::string r, std::string c, std::string b, std::string a,
     int lvl, int new_age, int new_weight, int c_hp, int m_hp, int t_hp, std::string h_dice, int str, int dex, int con, int intl, int wis, int cha, int init, int prof) {
    name = n;
    race = r;
    characterClass = c;
    background = b;
    alignment = a;
    level = lvl;
    hit_die_num = lvl;
    age = new_age;
    weight = new_weight;
    current_hp = c_hp;
    max_hp = m_hp;
    temp_hp = t_hp;
    deathSaveSuccesses = 0;
    deathSaveFailures = 0;
    hit_dice = h_dice;
    strength = str;
    dexterity = dex;
    constitution = con;
    intelligence = intl;
    wisdom = wis;
    charisma = cha;
    Initiative = init;
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
        f << name << "\n" << race << "\n" << characterClass << "\n"
          << background << "\n" << alignment << "\n"
          << level << " " << age << " " << weight << "\n"
          << current_hp << " " << max_hp << " " << temp_hp << "\n"
          << deathSaveSuccesses << " " << deathSaveFailures << "\n"
          << hit_dice << " " << hit_die_num << "\n"
          << strength << " " << dexterity << " " << constitution << " "
          << intelligence << " " << wisdom << " " << charisma << " "
          << Initiative << " " << proficiency << "\n"
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
    spellbook.saveSpellbook((fs::path(dir) / "spells.txt").string());
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
Character Character::loadFromDirectory(const std::string& dir) {
    namespace fs = std::filesystem;

    std::string name, race, characterClass, background, alignment, h_dice;
    int lvl = 1, age = 0, weight = 0;
    int c_hp = 0, m_hp = 0, t_hp = 0, h_dice_num = 0;
    int deathSuccesses = 0, deathFailures = 0;
    int str = 10, dex = 10, con = 10, intl = 10, wis = 10, cha = 10, init = 0, prof = 2;
    int armorIdx = -1, shieldIdx = -1;
    int insp = 0, spd = 30;
    int conditionCount = 0;
    std::vector<std::string> loadedConditions;

    {
        std::ifstream f((fs::path(dir) / "character.txt").string());
        if (!f) throw LoadError("cannot open character.txt in " + dir);
        std::getline(f, name);
        std::getline(f, race);
        std::getline(f, characterClass);
        std::getline(f, background);
        std::getline(f, alignment);
        f >> lvl >> age >> weight;
        f >> c_hp >> m_hp >> t_hp;
        f >> deathSuccesses >> deathFailures;
        f >> h_dice >> h_dice_num;
        f >> str >> dex >> con >> intl >> wis >> cha >> init >> prof;
        f >> armorIdx >> shieldIdx;
        f >> insp;
        f >> spd;
        if (f >> conditionCount)
        {
            f.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            for (int i = 0; i < conditionCount; i++)
            {
                std::string condition;
                std::getline(f, condition);
                if (!condition.empty())
                {
                    loadedConditions.push_back(condition);
                }
            }
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
        std::ifstream f((fs::path(dir) / "inventory.txt").string());
        if (f) c.getInventory().load(f);
    }
    {
        std::ifstream f((fs::path(dir) / "wallet.txt").string());
        if (f) c.getWallet().load(f);
    }
    c.getSpellbook().loadSpellbook((fs::path(dir) / "spells.txt").string());
    {
        std::ifstream f((fs::path(dir) / "spellslots.txt").string());
        if (f) c.getSpellSlots().load(f);
    }
    {
        std::ifstream f((fs::path(dir) / "features.txt").string());
        if (f) c.getFeatures().load(f);
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
    std::cout << "HP: "         << current_hp << "/" << max_hp;
    if (temp_hp > 0) std::cout << "  (+" << temp_hp << " temp)";
    std::cout << "\n";
    std::cout << "Death Saves: " << deathSaveSuccesses << " success, "
              << deathSaveFailures << " failure\n";
    std::cout << "AC: "         << getAC();
    if (equippedArmorIndex > 0 && equippedArmorIndex <= inventory.size())
        std::cout << "  [" << inventory.getItem(equippedArmorIndex).getName() << "]";
    if (equippedShieldIndex > 0 && equippedShieldIndex <= inventory.size())
        std::cout << " + Shield";
    std::cout << "\n";
    std::cout << "Hit Dice: "   << hit_die_num << "/" << level << hit_dice << "\n";
    std::cout << "STR: " << strength     << " (" << mod(strength)     << ")\n";
    std::cout << "DEX: " << dexterity    << " (" << mod(dexterity)    << ")\n";
    std::cout << "CON: " << constitution << " (" << mod(constitution) << ")\n";
    std::cout << "INT: " << intelligence << " (" << mod(intelligence) << ")\n";
    std::cout << "WIS: " << wisdom       << " (" << mod(wisdom)       << ")\n";
    std::cout << "CHA: " << charisma     << " (" << mod(charisma)     << ")\n";
    std::cout << "Initiative: +" << Initiative << "\n";
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
int Character::getCurrentHP() const{return current_hp;}
int Character::getMaxHP() const{return max_hp;}
int Character::getTempHP() const{return temp_hp;}
int Character::getDeathSaveSuccesses() const { return deathSaveSuccesses; }
int Character::getDeathSaveFailures() const { return deathSaveFailures; }
const std::vector<std::string>& Character::getConditions() const { return conditions; }
std::string Character::getHitDice() const{return hit_dice;}
int Character::getStrength() const {return strength;}
int Character::getDexterity() const {return dexterity;}
int Character::getConstitution() const {return constitution;}
int Character::getIntelligence() const {return intelligence;}
int Character::getWisdom() const {return wisdom;}
int Character::getCharisma() const {return charisma;}
int Character::getInitiative() const {return Initiative;}
int Character::getProficiency() const {return proficiency;}
int Character::getAbilityModifier(int& ability_score) {return (ability_score / 2) - 5;}

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
    current_hp = c_hp;
    // Any healing above 0 HP clears the death save track.
    if (current_hp > 0)
    {
        resetDeathSaves();
    }
}
void Character::setMaxHP(int m_hp){max_hp = m_hp;}
void Character::setTempHP(int t_hp){temp_hp = t_hp;}
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
void Character::setHitDice(const std::string& new_hit_dice){hit_dice = new_hit_dice;}
int Character::getHitDiceNum() const { return hit_die_num; }
void Character::setHitDiceNum(int n) { hit_die_num = (n < 0 ? 0 : (n > level ? level : n)); }


// Hit die utilities
void Character::spendHitDice(int count)
{
    if (count < 1 || count > hit_die_num) return;
    hit_die_num -= count;
}

void Character::recoverHitDice()
{
    // On a long rest, recover half the character's total hit dice (rounded up).
    int recovered = (level + 1) / 2;
    hit_die_num = (hit_die_num + recovered > level) ? level : hit_die_num + recovered;
}

// Setters Cont.
void Character::setStrength(int str) { strength = str; }
void Character::setDexterity(int dex) {dexterity = dex;}
void Character::setConstitution(int con) {constitution = con;}
void Character::setIntelligence(int intl) {intelligence = intl;}
void Character::setWisdom(int wis) {wisdom = wis;}
void Character::setCharisma(int cha) {charisma = cha;}
void Character::setInitiative(int init) {Initiative = init;}
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
// Spellbook
Spellbook& Character::getSpellbook()
{
    return spellbook;
}

SpellSlots& Character::getSpellSlots()
{
    return spellSlots;
}

void Character::showSpells() const
{
    std::cout << "\n=== Spellbook ===\n";
    spellbook.displayAllSpells();

    std::cout << "\n=== Spell Slots ===\n";
    spellSlots.displaySlots();
}
