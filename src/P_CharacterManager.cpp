#include "H_CharacterManager.h"
#include "H_Weapon.h"
#include "H_Armor.h"
#include "H_Gear.h"
#include "H_DiceRoller.h"
#include "H_DndExceptions.h"
#include <iostream>
#include <fstream>
#include <cctype>
#include <limits>
#include <filesystem>

// Main file for managing character utilities

// Indexing in c++ is 0-based, so first element in a container is element 0.
// But for display purposes it is converted to 1-based, and then converted back to 0-based for actual element referencing.

// All numeric values (damage, health, stats etc.) in DND are whole integers, though in the event that division occurs, such as when an attack does half damage 
// or a potion does half healing and the resulting value isnt a whole number, the numeric value is rounded up or down to a whole number at the Dungeon masters 
//discretion. This simplifies the numeric based data management, and means we dont ever have to use floats or doubles (yay)


// Anonymous namespace keeps these helpers private to this .cpp file.
namespace {

// Returns a lowercase copy so string comparisons can ignore letter casing.
std::string toLowerCopy(const std::string& value)
{
    std::string result = value;
    for (char& c : result)
    {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return result;
}

// Warlocks are the only class here whose spell slots reset on a short rest.
bool isWarlockClass(const Character& character)
{
    return toLowerCopy(character.getClass()) == "warlock";
}

D20Mode promptD20Mode(ConsoleIO& io)
{
    std::cout << "1. Normal\n";
    std::cout << "2. Advantage\n";
    std::cout << "3. Disadvantage\n";
    // Previously a bad entry silently fell back to Normal; now it re-prompts.
    switch (io.readInt("Choice: ", 1, 3))
    {
        case 2:  return D20Mode::Advantage;
        case 3:  return D20Mode::Disadvantage;
        default: return D20Mode::Normal;
    }
}

Spellbook loadGlobalSpellbook()
{
    Spellbook global;
    global.loadSpellbook("data/SpellBook.txt");
    return global;
}

void saveGlobalSpellbook(const Spellbook& spellbook)
{
    spellbook.saveSpellbook("data/SpellBook.txt");
}
}

CharacterManager::CharacterManager(ConsoleIO& io) : io(io) {}

void CharacterManager::createCharacter() {

    // Every prompt below re-prompts on bad input rather than falling through
    // with an unset or out-of-range value.
    const std::string name           = io.readName("Enter name: ");
    const std::string race           = io.readText("Enter race: ");
    const std::string characterClass = io.readText("Enter class: ");
    const std::string background     = io.readText("Enter background: ");
    const std::string alignment      = io.readText("Enter alignment: ");

    const int lvl    = io.readInt("Enter level (1-20): ", 1, 20);
    const int age    = io.readInt("Enter age: ", 0, 100000);
    const int weight = io.readInt("Enter weight: ", 0, 100000);

    const int m_hp = io.readInt("Enter max health: ", 1, 100000);
    const int c_hp = m_hp;
    const int t_hp = io.readInt("Enter temporary health (0 if none): ", 0, 100000);

    const std::string h_dice = io.readHitDice("Enter hit dice (e.g. d6, d8, d12): ");

    // Ability scores, in the fixed STR/DEX/CON/INT/WIS/CHA order.
    int new_AS[6];
    for (int i = 0; i < 6; i++)
    {
        new_AS[i] = io.readInt("Enter " + Ability_scores[i], 1, 20);
    }

    const int init = io.readInt("Enter initiative: ", -10, 20);
    // Proficiency bonus is +2 at level 1 and never lower; +6 is the level-20 cap.
    const int prof = io.readInt("Enter proficiency (2-6): ", 2, 6);

    // Build character using constructor and place into vector
    characters.emplace_back(name, race, characterClass, background, alignment,
                            lvl, age, weight, c_hp, m_hp, t_hp, h_dice,
                            new_AS[0], new_AS[1], new_AS[2],
                            new_AS[3], new_AS[4], new_AS[5], init, prof);

    std::cout << "Character created!\n";
}

bool CharacterManager::hasCharacters() const
{
    return !characters.empty();
}

int CharacterManager::selectCharacter(const std::string& prompt) const {
    if (characters.empty()) {
        std::cout << "No characters available.\n";
        return -1;
    }

    std::cout << "\n=== Characters ===\n";
    for (size_t i = 0; i < characters.size(); i++)
        std::cout << i + 1 << ". " << characters[i].getName() << "\n";
    std::cout << "0. Back\n";

    const int choice = io.readMenuChoice(prompt, static_cast<int>(characters.size()));
    return choice == 0 ? -1 : choice - 1;
}

void CharacterManager::viewCharacters() const {
    const int index = selectCharacter("Choice: ");
    if (index < 0) return;

    // Show all details of chosen character
    characters[index].display();
}

void CharacterManager::manageGlobalSpells()
{
    int choice = -1;

    do
    {
        std::cout << "\n=== Global Spells ===\n";
        std::cout << "1. View Spells By Level\n";
        std::cout << "2. Add Spell\n";
        std::cout << "3. Edit Spell\n";
        std::cout << "0. Back\n";
        choice = io.readMenuChoice("Choice: ", 3);

        if (choice == 1)
        {
            // Level 0 is cantrips.
            const int level = io.readInt("Enter spell level (0-9): ", 0, 9);

            Spellbook global = loadGlobalSpellbook();
            auto spells = global.getSpellsByLevel(level);

            if (spells.empty())
            {
                std::cout << "No spells found at that level.\n";
            }
            else
            {
                std::cout << "\n=== Spells At Level " << level << " ===\n";
                for (const auto& spell : spells)
                {
                    spell.DisplaySpellProperties();
                    std::cout << "------------------\n";
                }
            }
        }
        else if (choice == 2)
        {
            // Free-form fields use readName (printable, non-blank) so entries
            // like "1 action" or "60 ft" are accepted.
            const std::string name     = io.readName("Spell name: ");
            const std::string type     = io.readName("Type: ");
            const std::string effect   = io.readName("Effect: ");
            const int         level    = io.readInt("Level (0-9): ", 0, 9);
            const std::string time     = io.readName("Cast time: ");
            const std::string range    = io.readName("Range: ");
            const std::string comp     = io.readName("Components: ");
            const std::string duration = io.readName("Duration: ");
            const std::string save     = io.readName("Saving throw: ");
            const std::string desc     = io.readName("Description: ");

            Spellbook global = loadGlobalSpellbook();
            global.addSpell(Spell(name, type, effect, level, time, range, comp, duration, save, desc));
            saveGlobalSpellbook(global);
            std::cout << "Spell added to global spellbook!\n";
        }
        else if (choice == 3)
        {
            Spellbook global = loadGlobalSpellbook();
            auto spells = global.getAllSpells();

            if (spells.empty())
            {
                std::cout << "No spells available to edit.\n";
                continue;
            }

            global.displaySpellsWithIndex();
            std::cout << "0. Back\n";
            const int spellNum = io.readMenuChoice("Select spell number: ",
                                                   static_cast<int>(spells.size()));
            if (spellNum == 0) continue;

            Spell spellToEdit = spells[spellNum - 1];
            int editChoice = -1;

            do
            {
                std::cout << "\n=== Edit Spell: " << spellToEdit.getSpellName() << " ===\n";
                std::cout << "1. Name\n";
                std::cout << "2. Type\n";
                std::cout << "3. Effect\n";
                std::cout << "4. Level\n";
                std::cout << "5. Cast Time\n";
                std::cout << "6. Range\n";
                std::cout << "7. Components\n";
                std::cout << "8. Duration\n";
                std::cout << "9. Saving Throw\n";
                std::cout << "10. Description\n";
                std::cout << "11. View Spell Details\n";
                std::cout << "0. Save and Back\n";
                editChoice = io.readMenuChoice("Choice: ", 11);

                switch (editChoice)
                {
                    case 1:  spellToEdit.setSpellName(io.readName("Enter new spell name: ")); break;
                    case 2:  spellToEdit.setSpellType(io.readName("Enter new spell type: ")); break;
                    case 3:  spellToEdit.setSpellEffect(io.readName("Enter new spell effect: ")); break;
                    // Was getValidIntegerInput, which enforced > 0 and so could
                    // never set a cantrip back to level 0.
                    case 4:  spellToEdit.setSpellLevel(io.readInt("Enter new spell level (0-9): ", 0, 9)); break;
                    case 5:  spellToEdit.setSpellTime(io.readName("Enter new cast time: ")); break;
                    case 6:  spellToEdit.setSpellRange(io.readName("Enter new range: ")); break;
                    case 7:  spellToEdit.setSpellComponents(io.readName("Enter new components: ")); break;
                    case 8:  spellToEdit.setSpellDuration(io.readName("Enter new duration: ")); break;
                    case 9:  spellToEdit.setSpellSavingThrow(io.readName("Enter new saving throw: ")); break;
                    case 10: spellToEdit.setSpellDescription(io.readName("Enter new description: ")); break;
                    case 11: spellToEdit.DisplaySpellProperties(); break;
                    case 0:
                    {
                        if (global.updateSpell(static_cast<size_t>(spellNum - 1), spellToEdit))
                        {
                            saveGlobalSpellbook(global);
                            std::cout << "Spell updated.\n";
                        }
                        break;
                    }
                }
            } while (editChoice != 0);
        }
    } while (choice != 0);
}

void CharacterManager::editCharacter() {
    // Check for characters
    if (characters.empty()) {
        std::cout << "No characters to edit.\n";
        return;
    }

    const int index = selectCharacter("Enter character index: ");
    if (index < 0) return;

    // Assigns the character object chose to reference c to apply edits
    Character& c = characters[index];

    int choice;

    // Display edit menu
    do {
        std::cout << "\n=== " << c.getName() << " Editor ===\n";
        std::cout << "1. Character details \n2. Character health \n3. Inventory \n4. Ability scores \n5. Spells \n6. Features and skills \n0. Back \n";
        choice = io.readMenuChoice("Choice: ", 6);

        if (choice == 1) // Character details
        {
            int char_edit_choice;
            std::cout << "What would you like to change? " << std::endl;
            for (int i = 0; i < 9; i++)
            {
                std::cout << i + 1 << "." << EditCharDetailsArray[i] << std::endl;
            }
            char_edit_choice = io.readMenuChoice("Choice: ", 9);
            switch (char_edit_choice)
            {

            case 1: //Name
                {
                    std::string new_name = io.readName("Enter new Name: ");
                    c.setName(new_name);
                    break;
                }

            case 2: //Race
                {
                    std::string new_race = io.readText("Enter new Race: ");
                    c.setRace(new_race);
                    break;
                }
                  
            case 3: //Class
                {
                    std::string new_class = io.readText("Enter new Class: ");
                    c.setClass(new_class);
                    break;
                }
            
            case 4: //Background
                {
                    std::string new_background = io.readText("Enter new Background: ");
                    c.setBackground(new_background);
                    break;
                    
                }
            case 5: //Alignment
                {
                    std::string new_alignment = io.readText("Enter new Alignment: ");
                    c.setAlignment(new_alignment);
                    std::cout << "Alignment set" << std::endl;
                    break;
                }
            case 6: //Age
                {
                    int new_age = io.readInt("Enter new Age: ", 0, 100000);
                    c.setAge(new_age);
                    std::cout << "New age set" << std::endl;
                    break;
                }

            case 7: //Weight
                {
                    int new_weight = io.readInt("Enter new Weight: ", 0, 100000);
                    c.setWeight(new_weight);
                    std::cout << "New weight set" << std::endl;
                    break;
                }

            case 8: //Level
                {
                    c.setLevel(io.readInt("Enter new Level (1-20): ", 1, 20));
                    break;
                }

            case 9: //Speed
                {
                    int new_speed = io.readInt("Enter new Speed (ft): ", 0, 1000);
                    c.setSpeed(new_speed);
                    std::cout << "Speed set to " << new_speed << " ft\n";
                    break;
                }

            }

            
            
        }
        else if (choice == 2) // Health
        {
            int health_edit_choice;
            std::cout << "What would you like to change? " << std::endl;
            for (int i = 0; i < 7; i++)
            {
                std::cout << i + 1 << "." << EditHpArray[i] << std::endl;
            }
            health_edit_choice = io.readMenuChoice("Choice: ", 7);

            switch(health_edit_choice)
            {
                case 1: //Max Health
                {
                    int new_max_health = io.readInt("Enter new Max health: ", 1, 100000);
                    int max_hp_diff = c.getMaxHP() - new_max_health; 
              
                    if (new_max_health < c.getMaxHP()) // Check to se if max health is being reduced 
                    {
                        c.setMaxHP(new_max_health);

                        // Do Calc to adjust current health accordingly
                        int adjust_current_hp = c.getCurrentHP() - max_hp_diff;
                        c.setCurrentHP(adjust_current_hp);
                    }
                    else 
                    {
                        c.setMaxHP(new_max_health);
                    }
                    std::cout << "New max health set" << std::endl;

                    break;
                }
                case 2: //Current Health
                {
                 
                    // 0 is a valid current HP (unconscious); the old helper
                    // enforced > 0 and rejected it.
                    c.setCurrentHP(io.readInt(
                        "Enter new Current health (0-" + std::to_string(c.getMaxHP()) + "): ",
                        0, c.getMaxHP()));
                    break;
                }
                case 3: //Temporary Health
                {
                    int new_temp_health = io.readInt("Enter new Temporary Heath: ", 0, 100000);
                    c.setTempHP(new_temp_health);
                    std::cout << "New temp health set" << std::endl;
                    break;
                }
                case 4: //Hit Dice
                {
                    std::string new_hit_dice = io.readHitDice("Enter new hit dice (e.g. d6, d8, d12): ");
                    c.setHitDice(new_hit_dice);
                    std::cout << "New hit dice set" << std::endl;
                    break;
                }
                case 5: //Roll Death Save
                {
                    if (c.getCurrentHP() > 0)
                    {
                        std::cout << "Death saves are usually only needed at 0 HP.\n";
                    }

                    // Death saves explanation:
                    // In D&D, when a character is at 0 hit points and not stabilized or dead,
                    // they must make death saving throws to determine whether they move closer
                    // to stability or death. A roll of 10 or higher is a success; below 10 is
                    // a failure. Three successes means the character becomes stable. Three
                    // failures means the character dies. A natural 20 often grants immediate
                    // revival to 1 HP (handled by Character::applyDeathSaveRoll). A natural 1
                    // counts as two failures.

                    std::cout << "\n=== Death Save Roll ===\n";
                    const D20Mode mode = promptD20Mode(io);

                    DiceRoller roller;
                    const D20RollResult result = roller.rollD20(mode);

                    // Show both rolls when advantage/disadvantage is used so the chosen result is clear.
                    if (mode == D20Mode::Normal)
                    {
                        std::cout << "Roll: " << result.chosenRoll << "\n";
                    }
                    else
                    {
                        std::cout << "Rolls: " << result.firstRoll << ", " << result.secondRoll << "\n";
                        std::cout << "Chosen roll: " << result.chosenRoll << "\n";
                    }

                    if (result.chosenRoll >= 10)
                    {
                        std::cout << "Result: PASS\n";
                    }
                    else
                    {
                        std::cout << "Result: FAILURE\n";
                    }

                    const DeathSaveOutcome outcome = c.applyDeathSaveRoll(result.chosenRoll);

                    if (outcome == DeathSaveOutcome::Revived)
                    {
                        std::cout << "Natural 20: character regains 1 HP.\n";
                    }
                    else if (result.chosenRoll == 1)
                    {
                        std::cout << "Natural 1: counts as two failed death saves.\n";
                    }

                    if (outcome == DeathSaveOutcome::Stable)
                    {
                        std::cout << "You are stable.\n";
                    }
                    else if (outcome == DeathSaveOutcome::Dead)
                    {
                        std::cout << "You have died.\n";
                    }

                    std::cout << "Death saves: "
                              << c.getDeathSaveSuccesses() << " success, "
                              << c.getDeathSaveFailures() << " failure\n";
                    break;
                }
                case 6: //Reset Death Saves
                {
                    c.resetDeathSaves();
                    std::cout << "Death saves reset.\n";
                    break;
                }
                case 7: //Conditions
                {
                    int condition_choice = -1;
                    do
                    {
                        // Conditions explanation:
                        // Conditions represent temporary status effects applied to a character
                        // (e.g., stunned, poisoned, restrained). They may alter a character's
                        // capabilities, impose disadvantages, change movement, or have other
                        // gameplay impacts. This block allows adding, removing, and clearing
                        // those condition strings from the character's recorded list.

                        std::cout << "\n=== Conditions ===\n";
                        const auto& conditions = c.getConditions();
                        // Conditions are displayed with 1-based numbering because that matches user input.
                        if (conditions.empty())
                        {
                            std::cout << "No conditions recorded.\n";
                        }
                        else
                        {
                            for (size_t i = 0; i < conditions.size(); i++)
                            {
                                std::cout << i + 1 << ". " << conditions[i] << "\n";
                            }
                        }

                        std::cout << "1. Add Condition\n";
                        std::cout << "2. Remove Condition\n";
                        std::cout << "3. Clear Conditions\n";
                        std::cout << "0. Back\n";
                        condition_choice = io.readMenuChoice("Choice: ", 3);

                        if (condition_choice == 1)
                        {
                            std::string condition = io.readText("Enter new condition: ");
                            c.addCondition(condition);
                            std::cout << "Condition added.\n";
                        }
                        else if (condition_choice == 2)
                        {
                            const int index = io.readInt(
                                "Enter condition number: ", 1,
                                static_cast<int>(c.getConditions().size()));
                            if (c.removeCondition(index))
                            {
                                std::cout << "Condition removed.\n";
                            }
                            else
                            {
                                std::cout << "Invalid condition number.\n";
                            }
                        }
                        else if (condition_choice == 3)
                        {
                            c.clearConditions();
                            std::cout << "All conditions cleared.\n";
                        }
                        else if (condition_choice != 0)
                        {
                            std::cout << "Invalid choice.\n";
                        }
                    } while (condition_choice != 0);
                    break;
                }
            }

            
        }

        else if (choice == 3) // Inventory
        {
            manageInventory(c);
        }
        else if (choice == 4) // Stats
        {
            std::cout << "What stat would you like to change? "  << std::endl;
            for (int i = 0; i < 6; i++)
            {
                std::cout << i + 1 << ". " << Ability_scores[i] << std::endl;
            }
            // abil was previously unvalidated, so a value outside 1-6 indexed
            // Ability_scores out of bounds.
            const int abil = io.readInt("Choice: ", 1, 6);
            const int val  = io.readInt("New " + Ability_scores[abil - 1], 1, 20);
            c.setStats(val, abil);
        }
        else if (choice == 5) // Spells and hsort/long rest
        {
        int spellChoice;

            do {
                std::cout << "\n=== Character Spells ===\n";
                std::cout << "1. View Global Spells\n";
                std::cout << "2. Add Existing Spell to Character\n";
                std::cout << "3. View Character Spellbook\n";
                std::cout << "4. Cast Spell\n";
                std::cout << "5. Edit Spell Slots\n";
                std::cout << "6. Long Rest\n";
                std::cout << "7. Short Rest\n";
                std::cout << "0. Back\n";
                spellChoice = io.readMenuChoice("Choice: ", 7);

                if (spellChoice == 1)
                {
                    Spellbook global = loadGlobalSpellbook();
                    const int level = io.readInt("Enter spell level (0-9): ", 0, 9);

                    auto spells = global.getSpellsByLevel(level);

                    if (spells.empty())
                    {
                        std::cout << "No spells found at that level.\n";
                    }
                    else
                    {
                        for (size_t i = 0; i < spells.size(); i++)
                        {
                            std::cout << i + 1 << ". "
                                      << spells[i].getSpellName()
                                      << "\n";
                        }
                    }
                }
                else if (spellChoice == 2)
                {
                    Spellbook global = loadGlobalSpellbook();

                    const int level = io.readInt("Enter spell level to filter (0-9): ", 0, 9);

                    auto spells = global.getSpellsByLevel(level);

                    if (spells.empty())
                    {
                        std::cout << "No spells of that level.\n";
                        continue;
                    }

                    // Display Spells
                    std::cout << "\n=== Filtered Spells ===\n";
                    for (size_t i = 0; i < spells.size(); i++)
                    {
                        std::cout << i + 1 << ". "
                                << spells[i].getSpellName()
                                << " (Level " << spells[i].getSpellLevel() << ")\n";
                    }

                    std::cout << "0. Back\n";
                    const int spellNum = io.readMenuChoice("Select spell number: ",
                                                           static_cast<int>(spells.size()));
                    if (spellNum > 0)
                    {
                        c.getSpellbook().addSpell(spells[spellNum - 1]);
                        std::cout << "Spell added to character!\n";
                    }
                    else
                    {
                        std::cout << "Invalid selection.\n";
                    }
                }
                else if (spellChoice == 3)
                {
                    c.showSpells();
                }
                else if (spellChoice == 4)
                {
                    // Casting uses the character's own known spells, not the global spell list.
                    auto knownSpells = c.getSpellbook().getAllSpells();

                    if (knownSpells.empty())
                    {
                        std::cout << "Character has no spells in their spellbook.\n";
                        continue;
                    }

                    c.getSpellbook().displaySpellsWithIndex();
                    std::cout << "0. Back\n";
                    const int selectedSpell = io.readMenuChoice(
                        "Select spell number: ", static_cast<int>(knownSpells.size()));
                    if (selectedSpell == 0) continue;

                    const Spell& spellToCast = knownSpells[selectedSpell - 1];
                    const int spellLevel = spellToCast.getSpellLevel();

                    if (spellLevel == 0)
                    {
                        std::cout << spellToCast.getSpellName() << " is a cantrip and does not use a spell slot.\n";
                        continue;
                    }

                    // Allow upcasting, but never casting below the spell's base level.
                    const int slotLevel = io.readInt(
                        "Cast " + spellToCast.getSpellName() + " using what slot level? (" +
                        std::to_string(spellLevel) + "-9): ", spellLevel, 9);

                    if (c.getSpellSlots().useSlot(slotLevel))
                    {
                        std::cout << spellToCast.getSpellName() << " cast using a level "
                                  << slotLevel << " slot.\n";
                        std::cout << "Remaining level " << slotLevel << " slots: "
                                  << c.getSpellSlots().getCurrentSlots(slotLevel) << "/"
                                  << c.getSpellSlots().getMaxSlots(slotLevel) << "\n";
                    }
                    else
                    {
                        std::cout << "No level " << slotLevel << " spell slots remaining.\n";
                    }
                }
                else if (spellChoice == 5)
                {
                    int slotEditChoice = -1;

                    do
                    {
                        std::cout << "\n=== Edit Spell Slots ===\n";
                        c.getSpellSlots().displaySlots();
                        std::cout << "1. Set max slots for a level\n";
                        std::cout << "2. Set current slots for a level\n";
                        std::cout << "0. Back\n";
                        slotEditChoice = io.readMenuChoice("Choice: ", 2);

                        if (slotEditChoice == 1 || slotEditChoice == 2)
                        {
                            const int slotLevel = io.readInt("Spell level (1-9): ", 1, 9);

                            if (slotEditChoice == 1)
                            {
                                const int maxSlots = io.readInt("New max slots: ", 0, 20);

                                c.getSpellSlots().setSlots(slotLevel, maxSlots);
                                // Explicitly keep zero-max levels at zero current slots.
                                if (maxSlots == 0)
                                {
                                    c.getSpellSlots().setCurrentSlots(slotLevel, 0);
                                }
                                std::cout << "Max slots updated for level " << slotLevel << ".\n";
                            }
                            else
                            {
                                // Cannot exceed the max already configured for this level.
                                const int currentSlots = io.readInt(
                                    "New current slots: ", 0,
                                    c.getSpellSlots().getMaxSlots(slotLevel));

                                c.getSpellSlots().setCurrentSlots(slotLevel, currentSlots);
                                std::cout << "Current slots updated for level " << slotLevel << ".\n";
                            }
                        }
                    } while (slotEditChoice != 0);
                }
                else if (spellChoice == 6)
                {
                    c.getSpellSlots().resetSlots();
                    c.setCurrentHP(c.getMaxHP());
                    c.recoverHitDice();
                    std::cout << "Long rest complete. HP, spell slots, and hit dice restored.\n";
                    std::cout << "Hit dice: " << c.getHitDiceNum() << "/" << c.getLevel() << c.getHitDice() << "\n";
                }
                else if (spellChoice == 7)
                {
                    if (isWarlockClass(c))
                    {
                        c.getSpellSlots().resetSlots();
                        std::cout << "Short rest complete. Warlock spell slots restored to full.\n";
                    }
                    else
                    {
                        std::cout << "Short rest complete. Spell slots unchanged for "
                                  << c.getClass() << ".\n";
                    }

                    // All classes can spend hit dice during a short rest.
                    std::cout << "Hit dice available: " << c.getHitDiceNum()
                              << "/" << c.getLevel() << c.getHitDice() << "\n";
                    if (c.getHitDiceNum() > 0 && c.getCurrentHP() < c.getMaxHP())
                    {
                        const int toSpend = io.readInt(
                            "Spend how many hit dice to recover HP? (0 = No): ",
                            0, c.getHitDiceNum());

                        if (toSpend > 0)
                        {
                            const int hpGained = io.readInt(
                                "Enter total HP recovered (roll " + std::to_string(toSpend) +
                                c.getHitDice() + " + CON modifier per die): ", 0, 100000);
                            if (hpGained > 0)
                            {
                                int newHp = c.getCurrentHP() + hpGained;
                                c.setCurrentHP(newHp > c.getMaxHP() ? c.getMaxHP() : newHp);
                            }
                            c.spendHitDice(toSpend);
                            std::cout << "HP: " << c.getCurrentHP() << "/" << c.getMaxHP()
                                      << "  Hit dice remaining: " << c.getHitDiceNum() << "\n";
                        }
                    }
                }
            } while (spellChoice != 0);
        }
        else if (choice == 6) // Feats and abilites
        {
            manageFeatures(c);
        }
        else if (choice == 0)
        {
            std::cout << "Exiting edit menu " << std::endl;
        }

    } while (choice != 0);
}

// Save/Load functions
void CharacterManager::saveCharacter(const Character& c) const {
    namespace fs = std::filesystem;
    fs::path dir = fs::path("data") / "characters" / c.getName();
    try {
        fs::create_directories(dir);
        c.saveToDirectory(dir.string());
        std::cout << c.getName() << " saved.\n";
    } catch (const SaveError& e) {
        std::cout << "Failed to save " << c.getName() << ": " << e.what() << "\n";
    } catch (const fs::filesystem_error& e) {
        std::cout << "Filesystem error saving " << c.getName() << ": " << e.what() << "\n";
    }
}

void CharacterManager::saveAll() const {
    if (characters.empty()) {
        std::cout << "No characters to save.\n";
        return;
    }
    for (const auto& c : characters)
        saveCharacter(c);
}

void CharacterManager::loadAll() {
    namespace fs = std::filesystem;
    fs::path base = fs::path("data") / "characters";

    if (!fs::exists(base) || !fs::is_directory(base)) {
        std::cout << "No saved characters found.\n";
        return;
    }

    characters.clear();
    int loaded = 0;
    try {
        for (const auto& entry : fs::directory_iterator(base)) {
            if (entry.is_directory() && fs::exists(entry.path() / "character.txt")) {
                try {
                    characters.push_back(Character::loadFromDirectory(entry.path().string()));
                    loaded++;
                } catch (const LoadError& e) {
                    std::cout << "Skipped " << entry.path().filename().string()
                              << ": " << e.what() << "\n";
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cout << "Filesystem error reading character directory: " << e.what() << "\n";
    }

    if (loaded == 0)
        std::cout << "No saved characters found.\n";
    else
        std::cout << "Loaded " << loaded << " character(s).\n";
}

std::vector<std::string> CharacterManager::listCharacterNames() const {
    namespace fs = std::filesystem;
    std::vector<std::string> names;
    fs::path base = fs::path("data") / "characters";

    if (!fs::exists(base) || !fs::is_directory(base))
        return names;

    for (const auto& entry : fs::directory_iterator(base)) {
        if (entry.is_directory())
            names.push_back(entry.path().filename().string());
    }
    return names;
}

// Inventory
void CharacterManager::manageInventory(Character& c) {
    int choice;

    do {
        // Inventory menu
        std::cout << "\n=== Inventory ===\n";
        std::cout << "1. View\n";
        std::cout << "2. Add Item\n";
        std::cout << "3. Remove Item\n";
        std::cout << "4. Currency\n";
        std::cout << "5. Equip armor / shield\n";
        std::cout << "6. Unequip armor / shield\n";
        std::cout << "0. Back\n";
        choice = io.readMenuChoice("Choice: ", 6);

        if (choice == 1) {
            c.showInventory();
        }
        else if (choice == 2) // Create new item
        {
            std::cout << "Item type:\n1. Weapon\n2. Armor\n3. Gear\n";
            const int typeChoice = io.readInt("Choice: ", 1, 3);

            // Free-form descriptive fields use readName so entries like
            // "1d6" or "80/320 ft" are accepted.
            const std::string iName   = io.readName("Name: ");
            const std::string iDesc   = io.readName("Description: ");
            const std::string iRarity = io.readText("Rarity (Common/Uncommon/Rare/Very Rare/Legendary): ");
            const float       iWeight = io.readFloat("Weight (lb): ", 0.0f, 10000.0f);
            const int         iQty    = io.readInt("Quantity: ", 1, 100000);
            const int         iValue  = io.readInt("Value (gp): ", 0, 1000000);
            const bool        iAttune = io.readYesNo("Requires attunement? (y/n): ");

            if (typeChoice == 1) {
                const std::string dmgDice = io.readName("Damage dice (e.g. 1d6): ");
                const std::string dmgType = io.readName("Damage type (e.g. Slashing): ");
                const std::string wCat    = io.readName("Category (Simple/Martial): ");
                const std::string wType   = io.readName("Type (Melee/Ranged): ");
                const std::string props   = io.readName("Properties (e.g. Finesse, Light): ");
                const std::string range   = io.readName("Range (e.g. 5 ft / 80/320 ft): ");
                c.addItem(std::make_unique<Weapon>(iName, iDesc, iWeight, iQty, iValue, iRarity, iAttune,
                                                   dmgDice, dmgType, wCat, wType, props, range));
            }
            else if (typeChoice == 2) {
                const std::string aType      = io.readName("Armor type (Light/Medium/Heavy/Shield): ");
                const int         baseAC     = io.readInt("Base AC: ", 0, 30);
                const int         maxDex     = io.readInt("Max Dex bonus (-1 for no limit): ", -1, 10);
                const int         strReq     = io.readInt("Strength requirement (0 for none): ", 0, 30);
                const bool        stealthDis = io.readYesNo("Stealth disadvantage? (y/n): ");
                c.addItem(std::make_unique<Armor>(iName, iDesc, iWeight, iQty, iValue, iRarity, iAttune,
                                                  aType, baseAC, maxDex, strReq, stealthDis));
            }
            else {
                c.addItem(std::make_unique<Gear>(iName, iDesc, iWeight, iQty, iValue, iRarity, iAttune));
            }
            std::cout << "Item added.\n";
        }
        else if (choice == 3) // Remove item by index
        {
            if (c.getInventory().size() == 0) {
                std::cout << "Inventory is empty.\n";
            } else {
                c.showInventory();
                std::cout << "0. Back\n";
                const int index = io.readMenuChoice("Index to remove: ", c.getInventory().size());
                if (index > 0) c.removeItem(index);
            }
        }
        else if (choice == 5) // Equip armor or shield
        {
            if (c.getInventory().size() == 0) {
                std::cout << "Inventory is empty.\n";
            } else {
                c.showInventory();
                std::cout << "0. Cancel\n";
                const int idx = io.readMenuChoice("Enter item number to equip: ",
                                                  c.getInventory().size());
                if (idx > 0) {
                    c.equipArmor(idx);
                    c.equipShield(idx);
                    std::cout << "Equipped. AC is now " << c.getAC() << ".\n";
                }
            }
        }
        else if (choice == 6) // Unequip Armor or shield
        {
            std::cout << "1. Unequip armor\n2. Unequip shield\n0. Back\n";
            const int uChoice = io.readMenuChoice("Choice: ", 2);
            if (uChoice == 1) { c.unequipArmor();  std::cout << "Armor unequipped.\n"; }
            if (uChoice == 2) { c.unequipShield(); std::cout << "Shield unequipped.\n"; }
        }
        else if (choice == 4) // Manage currency
        {
            int currChoice;
            do {
                std::cout << "\n=== Currency ===\n";
                c.showCurrency();
                std::cout << "1. Add currency\n";
                std::cout << "2. Spend currency\n";
                std::cout << "0. Back\n";
                currChoice = io.readMenuChoice("Choice: ", 2);

                if (currChoice == 1 || currChoice == 2) {
                    const int pp = io.readInt("Platinum: ", 0, 1000000);
                    const int gp = io.readInt("Gold: ",     0, 1000000);
                    const int ep = io.readInt("Electrum: ", 0, 1000000);
                    const int sp = io.readInt("Silver: ",   0, 1000000);
                    const int cp = io.readInt("Copper: ",   0, 1000000);

                    int sign = (currChoice == 2) ? -1 : 1;
                    c.getWallet().adjustPlatinum(sign * pp);
                    c.getWallet().adjustGold(sign * gp);
                    c.getWallet().adjustElectrum(sign * ep);
                    c.getWallet().adjustSilver(sign * sp);
                    c.getWallet().adjustCopper(sign * cp);

                    std::cout << (currChoice == 1 ? "Currency added.\n" : "Currency spent.\n");
                }
            } while (currChoice != 0);
        }

    } while (choice != 0);
}

// Methods for managing features
void CharacterManager::manageFeatures(Character& c)
{
    int choice = -1;

    do
    {
        // This submenu acts as a lightweight tracker rather than a rules engine.
        std::cout << "\n=== Features And Skills ===\n";
        std::cout << "1. View all\n";
        std::cout << "2. Add feat\n";
        std::cout << "3. Remove feat\n";
        std::cout << "4. Add racial trait\n";
        std::cout << "5. Remove racial trait\n";
        std::cout << "6. View skills\n";
        std::cout << "7. Edit skill proficiency\n";
        std::cout << "8. Edit saving throw proficiency\n";
        std::cout << "9. Add language\n";
        std::cout << "10. Remove language\n";
        std::cout << "11. Toggle inspiration\n";
        std::cout << "0. Back\n";
        choice = io.readMenuChoice("Choice: ", 11);

        if (choice == 1) // View feats
        {
            c.showFeatures();
        }
        else if (choice == 2) // Add feat
        {
            c.getFeatures().addFeat(io.readName("Feat name: "));
            std::cout << "Feat added.\n";
        }
        else if (choice == 3) // Remove feat
        {
            std::cout << "\n=== Feats ===\n";
            c.getFeatures().displayFeats();
            const int index = io.readInt("Index to remove (0 to cancel): ", 0,
                                         static_cast<int>(c.getFeatures().getFeats().size()));
            if (index == 0) continue;

            if (c.getFeatures().removeFeat(index))
            {
                std::cout << "Feat removed.\n";
            }
            else
            {
                std::cout << "Invalid index.\n";
            }
        }
        else if (choice == 4) // Add racial feat/trait
        {
            c.getFeatures().addRacialTrait(io.readName("Racial trait name: "));
            std::cout << "Racial trait added.\n";
        }
        else if (choice == 5) // Remove racial feat/trait
        {
            std::cout << "\n=== Racial Traits ===\n";
            c.getFeatures().displayRacialTraits();
            const int index = io.readInt("Index to remove (0 to cancel): ", 0,
                                         static_cast<int>(c.getFeatures().getRacialTraits().size()));
            if (index == 0) continue;

            if (c.getFeatures().removeRacialTrait(index))
            {
                std::cout << "Racial trait removed.\n";
            }
            else
            {
                std::cout << "Invalid index.\n";
            }
        }
        else if (choice == 6) // View skills
        {
            std::cout << "\n=== Skills ===\n";
            c.getFeatures().displaySkills(c.getStrength(), c.getDexterity(), c.getConstitution(),
                                          c.getIntelligence(), c.getWisdom(), c.getCharisma(),
                                          c.getProficiency());
        }
        else if (choice == 7) // Edit skill proficiencies
        {
            const auto& skills = c.getFeatures().getSkills();
            // Skills are selected by index here, but stored internally by name.
            c.getFeatures().displaySkills(c.getStrength(), c.getDexterity(), c.getConstitution(),
                                          c.getIntelligence(), c.getWisdom(), c.getCharisma(),
                                          c.getProficiency());
            const int skillIndex = io.readInt("Skill number (0 to cancel): ", 0,
                                              static_cast<int>(skills.size()));
            if (skillIndex == 0) continue;

            const int rankValue = io.readInt("Rank (0=None, 1=Proficient, 2=Expertise): ", 0, 2);

            c.getFeatures().setSkillRank(skills[skillIndex - 1].name,
                                         static_cast<SkillRank>(rankValue));
            std::cout << "Skill updated.\n";
        }

        else if (choice == 8) // Edit saving throw proficiencys
        {
            std::cout << "\n=== Saving Throws ===\n";
            c.getFeatures().displaySaves(c.getStrength(), c.getDexterity(), c.getConstitution(),
                                         c.getIntelligence(), c.getWisdom(), c.getCharisma(),
                                         c.getProficiency());
            const std::string ability = io.readName(
                "Enter ability to toggle (STR/DEX/CON/INT/WIS/CHA) or 0 to cancel: ");
            if (ability != "0")
            {
                bool current = c.getFeatures().getSaveProficiency(ability);
                if (c.getFeatures().setSaveProficiency(ability, !current))
                    std::cout << ability << " saving throw " << (!current ? "proficient" : "not proficient") << ".\n";
                else
                    std::cout << "Invalid ability. Use STR, DEX, CON, INT, WIS, or CHA.\n";
            }
        }
        else if (choice == 9) // Add language
        {
            c.getFeatures().addLanguage(io.readName("Language name: "));
            std::cout << "Language added.\n";
        }
        else if (choice == 10) // Remove language
        {
            std::cout << "\n=== Languages ===\n";
            c.getFeatures().displayLanguages();
            const int index = io.readInt("Index to remove (0 to cancel): ", 0,
                                         static_cast<int>(c.getFeatures().getLanguages().size()));
            if (index == 0) continue;
            if (c.getFeatures().removeLanguage(index))
                std::cout << "Language removed.\n";
            else
                std::cout << "Invalid index.\n";
        }
        else if (choice == 11) // Toggle inspiration
        {
            c.toggleInspiration();
            std::cout << "\n=== Inspiration ===\n";
            std::cout << (c.getInspiration() ? "Yes" : "No") << "\n";
        }
    } while (choice != 0);
}
