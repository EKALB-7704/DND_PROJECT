#include "H_CharacterManager.h"
#include "H_ManagerHelpers.h"
#include "H_DndExceptions.h"
#include <iostream>
#include "H_DiceRoller.h"

// Character editor: HP, hit dice, death saves and conditions.

// HP, hit dice, death saves and conditions.
void CharacterManager::editHealth(Character& c)
{
    int health_edit_choice = -1;

    // Loops until 0, matching the inventory, spells and features
    // submenus. It used to act once and drop back to the editor.
    do
    {
        io.os() << "\nWhat would you like to change?\n";
        for (int i = 0; i < 7; i++)
        {
            io.os() << i + 1 << "." << editHpLabels[i] << std::endl;
        }
        io.os() << "0. Back\n";
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
                io.os() << "New max health set" << std::endl;

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
                io.os() << "New temp health set" << std::endl;
                break;
            }
            case 4: //Hit Dice
            {
                std::string new_hit_dice = io.readHitDice("Enter new hit dice (e.g. d6, d8, d12): ");
                c.setHitDice(new_hit_dice);
                io.os() << "New hit dice set" << std::endl;
                break;
            }
            case 5: //Roll Death Save
            {
                if (c.getCurrentHP() > 0)
                {
                    io.os() << "Death saves are usually only needed at 0 HP.\n";
                }

                // Death saves explanation:
                // In D&D, when a character is at 0 hit points and not stabilized or dead,
                // they must make death saving throws to determine whether they move closer
                // to stability or death. A roll of 10 or higher is a success; below 10 is
                // a failure. Three successes means the character becomes stable. Three
                // failures means the character dies. A natural 20 often grants immediate
                // revival to 1 HP (handled by Character::applyDeathSaveRoll). A natural 1
                // counts as two failures.

                io.os() << "\n=== Death Save Roll ===\n";
                const D20Mode mode = ManagerHelpers::promptD20Mode(io);

                DiceRoller roller;
                const D20RollResult result = roller.rollD20(mode);

                // Show both rolls when advantage/disadvantage is used so the chosen result is clear.
                if (mode == D20Mode::Normal)
                {
                    io.os() << "Roll: " << result.chosenRoll << "\n";
                }
                else
                {
                    io.os() << "Rolls: " << result.firstRoll << ", " << result.secondRoll << "\n";
                    io.os() << "Chosen roll: " << result.chosenRoll << "\n";
                }

                if (result.chosenRoll >= 10)
                {
                    io.os() << "Result: PASS\n";
                }
                else
                {
                    io.os() << "Result: FAILURE\n";
                }

                const DeathSaveOutcome outcome = c.applyDeathSaveRoll(result.chosenRoll);

                if (outcome == DeathSaveOutcome::Revived)
                {
                    io.os() << "Natural 20: character regains 1 HP.\n";
                }
                else if (result.chosenRoll == 1)
                {
                    io.os() << "Natural 1: counts as two failed death saves.\n";
                }

                if (outcome == DeathSaveOutcome::Stable)
                {
                    io.os() << "You are stable.\n";
                }
                else if (outcome == DeathSaveOutcome::Dead)
                {
                    io.os() << "You have died.\n";
                }

                io.os() << "Death saves: "
                          << c.getDeathSaveSuccesses() << " success, "
                          << c.getDeathSaveFailures() << " failure\n";
                break;
            }
            case 6: //Reset Death Saves
            {
                c.resetDeathSaves();
                io.os() << "Death saves reset.\n";
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

                    io.os() << "\n=== Conditions ===\n";
                    const auto& conditions = c.getConditions();
                    // Conditions are displayed with 1-based numbering because that matches user input.
                    if (conditions.empty())
                    {
                        io.os() << "No conditions recorded.\n";
                    }
                    else
                    {
                        for (size_t i = 0; i < conditions.size(); i++)
                        {
                            io.os() << i + 1 << ". " << conditions[i] << "\n";
                        }
                    }

                    io.os() << "1. Add Condition\n";
                    io.os() << "2. Remove Condition\n";
                    io.os() << "3. Clear Conditions\n";
                    io.os() << "0. Back\n";
                    condition_choice = io.readMenuChoice("Choice: ", 3);

                    if (condition_choice == 1)
                    {
                        std::string condition = io.readText("Enter new condition: ");
                        c.addCondition(condition);
                        io.os() << "Condition added.\n";
                    }
                    else if (condition_choice == 2)
                    {
                        const int index = io.readInt(
                            "Enter condition number: ", 1,
                            static_cast<int>(c.getConditions().size()));
                        if (c.removeCondition(index))
                        {
                            io.os() << "Condition removed.\n";
                        }
                        else
                        {
                            io.os() << "Invalid condition number.\n";
                        }
                    }
                    else if (condition_choice == 3)
                    {
                        c.clearConditions();
                        io.os() << "All conditions cleared.\n";
                    }
                    else if (condition_choice != 0)
                    {
                        io.os() << "Invalid choice.\n";
                    }
                } while (condition_choice != 0);
                break;
            }
        }
    } while (health_edit_choice != 0);
}

// The six ability scores.
