#include "H_CharacterManager.h"
#include "H_ManagerHelpers.h"
#include "H_DndExceptions.h"
#include <iostream>

// The global spell registry, shared by every character.

void CharacterManager::manageGlobalSpells()
{
    int choice = -1;

    do
    {
        io.os() << "\n=== Global Spells ===\n";
        io.os() << "1. View Spells By Level\n";
        io.os() << "2. Add Spell\n";
        io.os() << "3. Edit Spell\n";
        io.os() << "0. Back\n";
        choice = io.readMenuChoice("Choice: ", 3);

        if (choice == 1)
        {
            // Level 0 is cantrips.
            const int level = io.readInt("Enter spell level (0-9): ", 0, 9);

            SpellBook global = ManagerHelpers::loadGlobalSpellBook(dataDir);
            auto spells = global.getSpellsByLevel(level);

            if (spells.empty())
            {
                io.os() << "No spells found at that level.\n";
            }
            else
            {
                io.os() << "\n=== Spells At Level " << level << " ===\n";
                for (const auto& spell : spells)
                {
                    spell.DisplaySpellProperties();
                    io.os() << "------------------\n";
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

            SpellBook global = ManagerHelpers::loadGlobalSpellBook(dataDir);
            global.addSpell(Spell(name, type, effect, level, time, range, comp, duration, save, desc));
            ManagerHelpers::saveGlobalSpellBook(global, dataDir);
            io.os() << "Spell added to global spellbook!\n";
        }
        else if (choice == 3)
        {
            SpellBook global = ManagerHelpers::loadGlobalSpellBook(dataDir);
            auto spells = global.getAllSpells();

            if (spells.empty())
            {
                io.os() << "No spells available to edit.\n";
                continue;
            }

            global.displaySpellsWithIndex();
            io.os() << "0. Back\n";
            const int spellNum = io.readMenuChoice("Select spell number: ",
                                                   static_cast<int>(spells.size()));
            if (spellNum == 0) continue;

            Spell spellToEdit = spells[spellNum - 1];
            int editChoice = -1;

            do
            {
                io.os() << "\n=== Edit Spell: " << spellToEdit.getSpellName() << " ===\n";
                io.os() << "1. Name\n";
                io.os() << "2. Type\n";
                io.os() << "3. Effect\n";
                io.os() << "4. Level\n";
                io.os() << "5. Cast Time\n";
                io.os() << "6. Range\n";
                io.os() << "7. Components\n";
                io.os() << "8. Duration\n";
                io.os() << "9. Saving Throw\n";
                io.os() << "10. Description\n";
                io.os() << "11. View Spell Details\n";
                io.os() << "0. Save and Back\n";
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
                            ManagerHelpers::saveGlobalSpellBook(global, dataDir);
                            io.os() << "Spell updated.\n";
                        }
                        break;
                    }
                }
            } while (editChoice != 0);
        }
    } while (choice != 0);
}

// Character details -- name, race, class, level, speed and so on.
