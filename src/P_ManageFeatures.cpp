#include "H_CharacterManager.h"
#include "H_DndExceptions.h"
#include <iostream>

// Character editor: feats, racial traits, languages, skills and saves.

// Methods for managing features
void CharacterManager::manageFeatures(Character& c)
{
    int choice = -1;

    do
    {
        // This submenu acts as a lightweight tracker rather than a rules engine.
        io.os() << "\n=== Features And Skills ===\n";
        io.os() << "1. View all\n";
        io.os() << "2. Add feat\n";
        io.os() << "3. Remove feat\n";
        io.os() << "4. Add racial trait\n";
        io.os() << "5. Remove racial trait\n";
        io.os() << "6. View skills\n";
        io.os() << "7. Edit skill proficiency\n";
        io.os() << "8. Edit saving throw proficiency\n";
        io.os() << "9. Add language\n";
        io.os() << "10. Remove language\n";
        io.os() << "11. Toggle inspiration\n";
        io.os() << "0. Back\n";
        choice = io.readMenuChoice("Choice: ", 11);

        if (choice == 1) // View feats
        {
            c.showFeatures();
        }
        else if (choice == 2) // Add feat
        {
            c.getFeatures().addFeat(io.readName("Feat name: "));
            io.os() << "Feat added.\n";
        }
        else if (choice == 3) // Remove feat
        {
            io.os() << "\n=== Feats ===\n";
            c.getFeatures().displayFeats();
            const int index = io.readInt("Index to remove (0 to cancel): ", 0,
                                         static_cast<int>(c.getFeatures().getFeats().size()));
            if (index == 0) continue;

            if (c.getFeatures().removeFeat(index))
            {
                io.os() << "Feat removed.\n";
            }
            else
            {
                io.os() << "Invalid index.\n";
            }
        }
        else if (choice == 4) // Add racial feat/trait
        {
            c.getFeatures().addRacialTrait(io.readName("Racial trait name: "));
            io.os() << "Racial trait added.\n";
        }
        else if (choice == 5) // Remove racial feat/trait
        {
            io.os() << "\n=== Racial Traits ===\n";
            c.getFeatures().displayRacialTraits();
            const int index = io.readInt("Index to remove (0 to cancel): ", 0,
                                         static_cast<int>(c.getFeatures().getRacialTraits().size()));
            if (index == 0) continue;

            if (c.getFeatures().removeRacialTrait(index))
            {
                io.os() << "Racial trait removed.\n";
            }
            else
            {
                io.os() << "Invalid index.\n";
            }
        }
        else if (choice == 6) // View skills
        {
            io.os() << "\n=== Skills ===\n";
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
            io.os() << "Skill updated.\n";
        }

        else if (choice == 8) // Edit saving throw proficiencys
        {
            io.os() << "\n=== Saving Throws ===\n";
            c.getFeatures().displaySaves(c.getStrength(), c.getDexterity(), c.getConstitution(),
                                         c.getIntelligence(), c.getWisdom(), c.getCharisma(),
                                         c.getProficiency());
            const std::string ability = io.readName(
                "Enter ability to toggle (STR/DEX/CON/INT/WIS/CHA) or 0 to cancel: ");
            if (ability != "0")
            {
                bool current = c.getFeatures().getSaveProficiency(ability);
                if (c.getFeatures().setSaveProficiency(ability, !current))
                    io.os() << ability << " saving throw " << (!current ? "proficient" : "not proficient") << ".\n";
                else
                    io.os() << "Invalid ability. Use STR, DEX, CON, INT, WIS, or CHA.\n";
            }
        }
        else if (choice == 9) // Add language
        {
            c.getFeatures().addLanguage(io.readName("Language name: "));
            io.os() << "Language added.\n";
        }
        else if (choice == 10) // Remove language
        {
            io.os() << "\n=== Languages ===\n";
            c.getFeatures().displayLanguages();
            const int index = io.readInt("Index to remove (0 to cancel): ", 0,
                                         static_cast<int>(c.getFeatures().getLanguages().size()));
            if (index == 0) continue;
            if (c.getFeatures().removeLanguage(index))
                io.os() << "Language removed.\n";
            else
                io.os() << "Invalid index.\n";
        }
        else if (choice == 11) // Toggle inspiration
        {
            c.toggleInspiration();
            io.os() << "\n=== Inspiration ===\n";
            io.os() << (c.getInspiration() ? "Yes" : "No") << "\n";
        }
    } while (choice != 0);
}
