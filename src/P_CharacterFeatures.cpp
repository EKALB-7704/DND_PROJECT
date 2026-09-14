#include "H_CharacterFeatures.h"
#include "H_SaveFormat.h"
#include "H_DndExceptions.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <limits>

// Probably should explain the difference between skills and feats in DND.

// Basically, Skills are universal components of DND characters, every character has the same ones.
// These skills represent how capable your character is at certain tasks and when attempting to do something 
// you may be asked to make a "skill check". this means you must roll a d20 to determine if you succeed or fail
// What skills are used for any given task or action is decided by the Dungeon master

// Being certain classes, races and backgrounds can cause you to become "proficient" in a skill, 
// which means you can add your proficiency bonus along with your ability score modifier
// to a roll when using that particular skill for a check.

// at some point you may gain "expertise" in a skill, which allows you to double your prof bonus 
// before adding it to your roll. For example a character "proficient" in "Athletics" who rolled a 14
// would have a final value for an "Athletics" check like this: 14 + strength mod + prof bonus = final value 

// if that same character rolled a 13 but has "expertise" in Athletics,
// then the roll would be: 13 + strength mod + (prof bonus x 2) = final value

// which ability score (AS) modifier you add to your roll for a skill check is dependent on which AS it
// is derived from (This is predetermined by the rules of DND 5e). For example, "Athletics" is 
// derived from the strength AS and as such the Strength modifier is used when making a check with this skill

// Feats are able to be chosen at specific levels and are essentiallty additional abilites not dependent 
// on your class. "Racial" feats are a specific type of feat gained at level 1 upon character 
// creation and ,as named, are determined by your race  

namespace {

// Normalizes text for case-insensitive skill lookups.
std::string toLowerCopy(const std::string& value)
{
    std::string result = value;
    for (char& c : result)
    {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return result;
}

// Standard 5e ability modifier formula.
int abilityModifier(int score)
{
    return (score / 2) - 5;
}

// Maps a skill's linked ability code to the character's matching ability score.
int scoreForAbility(const std::string& ability,
                    int strength, int dexterity, int constitution,
                    int intelligence, int wisdom, int charisma)
{
    if (ability == "STR") return strength;
    if (ability == "DEX") return dexterity;
    if (ability == "CON") return constitution;
    if (ability == "INT") return intelligence;
    if (ability == "WIS") return wisdom;
    return charisma;
}
}

// Character features track skill ranks, saves, feats, traits, and languages.

// Saving throws:

// At some point during a DND campaign you will be asked to make a "Saving throw".
// This is effectively a specific type of check to see whether an action or occurance affects you,
// such as avoidng a trap that was triggered or breaking out a grapple attempt. 

// you can be "proficient" in a particular type of saving throw and, similarly to skill checks, you can add your prof bonus to your
// roll. However, Unlike skill checks, saving throws only ever use the base Ability scores: STR, DEX, CON, INT, WIS, CHA.

// There is a special type of saving throw called a "Death save". This occurs when a character reaches 0 hp but 
// is not killed outright. For every consecutive turn while your health remains at 0, you must roll a d20 to determine if you pass or fail.

// Death saves use no modifiers and have a simple pass requirment of 10 or higher using a d20 dice.
// 3 successes ( 3 rolls of 10+, or a single natural 20 roll) "stabilizes" your character at 1 hp. 
// 3 fails (9 or lower) causes your character to die. getting a natural 1 counts as 2 fails

// you can also be stabilized by being healed by outside sources

CharacterFeatures::CharacterFeatures()
    : skills({
        // Initialize the standard 5e skill list with no proficiencies selected yet.
        {"Acrobatics", "DEX", SkillRank::None},
        {"Animal Handling", "WIS", SkillRank::None},
        {"Arcana", "INT", SkillRank::None},
        {"Athletics", "STR", SkillRank::None},
        {"Deception", "CHA", SkillRank::None},
        {"History", "INT", SkillRank::None},
        {"Insight", "WIS", SkillRank::None},
        {"Intimidation", "CHA", SkillRank::None},
        {"Investigation", "INT", SkillRank::None},
        {"Medicine", "WIS", SkillRank::None},
        {"Nature", "INT", SkillRank::None},
        {"Perception", "WIS", SkillRank::None},
        {"Performance", "CHA", SkillRank::None},
        {"Persuasion", "CHA", SkillRank::None},
        {"Religion", "INT", SkillRank::None},
        {"Sleight of Hand", "DEX", SkillRank::None},
        {"Stealth", "DEX", SkillRank::None},
        {"Survival", "WIS", SkillRank::None}
    })
    , savingThrows({
        {"STR", false},
        {"DEX", false},
        {"CON", false},
        {"INT", false},
        {"WIS", false},
        {"CHA", false}
    })
{
}

// Find a mutable skill entry by name so menus can edit it.
SkillEntry* CharacterFeatures::findSkill(const std::string& skillName)
{
    const std::string target = toLowerCopy(skillName);
    for (auto& skill : skills)
    {
        if (toLowerCopy(skill.name) == target)
        {
            return &skill;
        }
    }
    return nullptr;
}

// Find a read-only skill entry by name for calculations and display.
const SkillEntry* CharacterFeatures::findSkill(const std::string& skillName) const
{
    const std::string target = toLowerCopy(skillName);
    for (const auto& skill : skills)
    {
        if (toLowerCopy(skill.name) == target)
        {
            return &skill;
        }
    }
    return nullptr;
}

// Feats are stored as simple text entries.
void CharacterFeatures::addFeat(const std::string& feat)
{
    if (!feat.empty())
    {
        feats.push_back(feat);
    }
}

bool CharacterFeatures::removeFeat(int index)
{
    if (index < 1 || index > static_cast<int>(feats.size()))
    {
        return false;
    }

    feats.erase(feats.begin() + index - 1);
    return true;
}

const std::vector<std::string>& CharacterFeatures::getFeats() const
{
    return feats;
}

// Racial traits are tracked separately from general feats.
void CharacterFeatures::addRacialTrait(const std::string& trait)
{
    if (!trait.empty())
    {
        racialTraits.push_back(trait);
    }
}

bool CharacterFeatures::removeRacialTrait(int index)
{
    if (index < 1 || index > static_cast<int>(racialTraits.size()))
    {
        return false;
    }

    racialTraits.erase(racialTraits.begin() + index - 1);
    return true;
}

const std::vector<std::string>& CharacterFeatures::getRacialTraits() const
{
    return racialTraits;
}

// Languages are stored as a basic list for display and save/load.
void CharacterFeatures::addLanguage(const std::string& language)
{
    if (!language.empty())
        languages.push_back(language);
}

bool CharacterFeatures::removeLanguage(int index)
{
    if (index < 1 || index > static_cast<int>(languages.size()))
        return false;
    languages.erase(languages.begin() + index - 1);
    return true;
}

const std::vector<std::string>& CharacterFeatures::getLanguages() const
{
    return languages;
}

void CharacterFeatures::displayLanguages() const
{
    if (languages.empty())
    {
        std::cout << "No languages recorded.\n";
        return;
    }
    for (size_t i = 0; i < languages.size(); i++)
        std::cout << i + 1 << ". " << languages[i] << "\n";
}

// Skill ranks control whether a skill gets no bonus, proficiency, or expertise.
bool CharacterFeatures::setSkillRank(const std::string& skillName, SkillRank rank)
{
    SkillEntry* skill = findSkill(skillName);
    if (skill == nullptr)
    {
        return false;
    }

    skill->rank = rank;
    return true;
}

SkillRank CharacterFeatures::getSkillRank(const std::string& skillName) const
{
    const SkillEntry* skill = findSkill(skillName);
    return (skill != nullptr) ? skill->rank : SkillRank::None;
}

const std::vector<SkillEntry>& CharacterFeatures::getSkills() const
{
    return skills;
}

// Calculate the final modifier for one skill check.
int CharacterFeatures::getSkillModifier(const std::string& skillName,
                                        int strength, int dexterity, int constitution,
                                        int intelligence, int wisdom, int charisma,
                                        int proficiencyBonus) const
{
    const SkillEntry* skill = findSkill(skillName);
    if (skill == nullptr)
    {
        return 0;
    }

    // Skill bonus = linked ability modifier + proficiency or expertise bonus.
    int modifier = abilityModifier(scoreForAbility(skill->ability,
                                                   strength, dexterity, constitution,
                                                   intelligence, wisdom, charisma));

    if (skill->rank == SkillRank::Proficient)
    {
        modifier += proficiencyBonus;
    }
    else if (skill->rank == SkillRank::Expertise)
    {
        modifier += proficiencyBonus * 2;
    }

    return modifier;
}

void CharacterFeatures::displayFeats() const
{
    if (feats.empty())
    {
        std::cout << "No feats recorded.\n";
        return;
    }

    for (size_t i = 0; i < feats.size(); i++)
    {
        std::cout << i + 1 << ". " << feats[i] << "\n";
    }
}

void CharacterFeatures::displayRacialTraits() const
{
    if (racialTraits.empty())
    {
        std::cout << "No racial traits recorded.\n";
        return;
    }

    for (size_t i = 0; i < racialTraits.size(); i++)
    {
        std::cout << i + 1 << ". " << racialTraits[i] << "\n";
    }
}

void CharacterFeatures::displaySkills(int strength, int dexterity, int constitution,
                                      int intelligence, int wisdom, int charisma,
                                      int proficiencyBonus) const
{
    for (size_t i = 0; i < skills.size(); i++)
    {
        const SkillEntry& skill = skills[i];
        const int modifier = getSkillModifier(skill.name,
                                              strength, dexterity, constitution,
                                              intelligence, wisdom, charisma,
                                              proficiencyBonus);
        const std::string rankLabel =
            (skill.rank == SkillRank::Expertise) ? "Expertise" :
            (skill.rank == SkillRank::Proficient) ? "Proficient" : "None";

        std::cout << i + 1 << ". " << skill.name
                  << " [" << skill.ability << "] "
                  << (modifier >= 0 ? "+" : "") << modifier
                  << " - " << rankLabel << "\n";
    }
    
}

// Save all feature data so it can be restored with the character later.
void CharacterFeatures::save(std::ofstream& file) const
{
    // Save the two free-text lists first, then each skill's current rank.
    SaveFormat::writeHeader(file, SaveFormat::kFeaturesTag);
    file << feats.size() << "\n";
    for (const auto& feat : feats)
    {
        file << feat << "\n";
    }

    file << racialTraits.size() << "\n";
    for (const auto& trait : racialTraits)
    {
        file << trait << "\n";
    }

    file << skills.size() << "\n";
    for (const auto& skill : skills)
    {
        file << skill.name << "\n";
        file << skill.ability << "\n";
        file << static_cast<int>(skill.rank) << "\n";
    }
    for (const auto& s : savingThrows)
        file << (s.proficient ? 1 : 0) << "\n";

    file << languages.size() << "\n";
    for (const auto& lang : languages)
        file << lang << "\n";
}

// Load all saved feature data back into the built-in feature tables.
void CharacterFeatures::load(std::ifstream& file)
{
    feats.clear();
    racialTraits.clear();

    SaveFormat::readHeader(file, SaveFormat::kFeaturesTag, "features.txt");

    std::string line;

    const int featCount = SaveFormat::readCount(file, "features.txt (feats)");
    for (int i = 0; i < featCount; i++)
    {
        if (!SaveFormat::readLine(file, line))
            throw LoadError("features.txt ended mid-way through the feat list");
        addFeat(line);
    }

    const int traitCount = SaveFormat::readCount(file, "features.txt (racial traits)");
    for (int i = 0; i < traitCount; i++)
    {
        if (!SaveFormat::readLine(file, line))
            throw LoadError("features.txt ended mid-way through the racial trait list");
        addRacialTrait(line);
    }

    const int skillCount = SaveFormat::readCount(file, "features.txt (skills)");
    for (int i = 0; i < skillCount; i++)
    {
        std::string name, ability, rankLine;
        if (!SaveFormat::readLine(file, name) ||
            !SaveFormat::readLine(file, ability) ||
            !SaveFormat::readLine(file, rankLine))
        {
            throw LoadError("features.txt ended mid-way through the skill list");
        }

        int rankValue = 0;
        if (!SaveFormat::parseWholeInt(rankLine, rankValue))
            throw LoadError("unreadable skill rank in features.txt: \"" + rankLine + "\"");
        // Ranks are None/Proficient/Expertise; anything else means no training.
        if (rankValue < 0 || rankValue > 2) rankValue = 0;

        SkillEntry* skill = findSkill(name);
        if (skill != nullptr)
        {
            // Reapply the saved state onto the built-in 5e skill table.
            skill->ability = ability;
            skill->rank = static_cast<SkillRank>(rankValue);
        }
    }

    // One flag per saving throw, in the fixed order built by the constructor.
    for (auto& sv : savingThrows)
    {
        if (!SaveFormat::readLine(file, line)) break;
        int val = 0;
        if (!SaveFormat::parseWholeInt(line, val))
            throw LoadError("unreadable saving throw flag in features.txt");
        sv.proficient = (val != 0);
    }

    // Languages were added after the original format, so a file that stops
    // here is a valid older save rather than a truncated one.
    languages.clear();
    if (SaveFormat::readLine(file, line))
    {
        int langCount = 0;
        if (!SaveFormat::parseWholeInt(line, langCount) ||
            langCount < 0 || langCount > SaveFormat::kMaxRecords)
        {
            throw LoadError("unreadable language count in features.txt");
        }
        for (int i = 0; i < langCount; i++)
        {
            if (!SaveFormat::readLine(file, line))
                throw LoadError("features.txt ended mid-way through the language list");
            addLanguage(line);
        }
    }
}

// Saving throws are stored separately from the skill list.
SaveEntry* CharacterFeatures::findSave(const std::string& ability)
{
    for (auto& s : savingThrows)
        if (s.ability == ability) return &s;
    return nullptr;
}

const SaveEntry* CharacterFeatures::findSave(const std::string& ability) const
{
    for (const auto& s : savingThrows)
        if (s.ability == ability) return &s;
    return nullptr;
}

bool CharacterFeatures::setSaveProficiency(const std::string& ability, bool proficient)
{
    SaveEntry* s = findSave(ability);
    if (!s) return false;
    s->proficient = proficient;
    return true;
}

bool CharacterFeatures::getSaveProficiency(const std::string& ability) const
{
    const SaveEntry* s = findSave(ability);
    return s ? s->proficient : false;
}

const std::vector<SaveEntry>& CharacterFeatures::getSavingThrows() const
{
    return savingThrows;
}

// Calculate the final modifier for one saving throw.
int CharacterFeatures::getSaveModifier(const std::string& ability,
                                        int strength, int dexterity, int constitution,
                                        int intelligence, int wisdom, int charisma,
                                        int proficiencyBonus) const
{
    const SaveEntry* s = findSave(ability);
    if (!s) return 0;
    int mod = abilityModifier(scoreForAbility(ability,
        strength, dexterity, constitution, intelligence, wisdom, charisma));
    if (s->proficient) mod += proficiencyBonus;
    return mod;
}                                        

void CharacterFeatures::displaySaves(int strength, int dexterity, int constitution,
                                     int intelligence, int wisdom, int charisma,
                                     int proficiencyBonus) const
{
    for (const auto& s : savingThrows) {
        int mod = getSaveModifier(s.ability,
            strength, dexterity, constitution, intelligence, wisdom, charisma,
            proficiencyBonus);
        std::cout << (s.proficient ? "[P] " : "[ ] ")
                  << s.ability << ": "
                  << (mod >= 0 ? "+" : "") << mod << "\n";
    }
}
