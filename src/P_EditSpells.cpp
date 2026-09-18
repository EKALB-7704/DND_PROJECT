#include "H_CharacterManager.h"
#include "H_ManagerHelpers.h"
#include "H_DndExceptions.h"
#include <iostream>

// Character editor: spellbook, spell slots, and short/long rests.

// Character spellbook, spell slots, and short/long rests.
void CharacterManager::editSpells(Character& c)
{
int spellChoice;

    do {
        io.os() << "\n=== Character Spells ===\n";
        io.os() << "1. View Global Spells\n";
        io.os() << "2. Add Existing Spell to Character\n";
        io.os() << "3. View Character SpellBook\n";
        io.os() << "4. Cast Spell\n";
        io.os() << "5. Edit Spell Slots\n";
        io.os() << "0. Back\n";
        spellChoice = io.readMenuChoice("Choice: ", 5);

        if (spellChoice == 1)
        {
            SpellBook global = ManagerHelpers::loadGlobalSpellBook(dataDir);
            const int level = io.readInt("Enter spell level (0-9): ", 0, 9);

            auto spells = global.getSpellsByLevel(level);

            if (spells.empty())
            {
                io.os() << "No spells found at that level.\n";
            }
            else
            {
                for (size_t i = 0; i < spells.size(); i++)
                {
                    io.os() << i + 1 << ". "
                              << spells[i].getSpellName()
                              << "\n";
                }
            }
        }
        else if (spellChoice == 2)
        {
            SpellBook global = ManagerHelpers::loadGlobalSpellBook(dataDir);

            const int level = io.readInt("Enter spell level to filter (0-9): ", 0, 9);

            auto spells = global.getSpellsByLevel(level);

            if (spells.empty())
            {
                io.os() << "No spells of that level.\n";
                continue;
            }

            // Display Spells
            io.os() << "\n=== Filtered Spells ===\n";
            for (size_t i = 0; i < spells.size(); i++)
            {
                io.os() << i + 1 << ". "
                        << spells[i].getSpellName()
                        << " (Level " << spells[i].getSpellLevel() << ")\n";
            }

            io.os() << "0. Back\n";
            const int spellNum = io.readMenuChoice("Select spell number: ",
                                                   static_cast<int>(spells.size()));
            if (spellNum > 0)
            {
                c.getSpellBook().addSpell(spells[spellNum - 1]);
                io.os() << "Spell added to character!\n";
            }
            else
            {
                io.os() << "Invalid selection.\n";
            }
        }
        else if (spellChoice == 3)
        {
            c.showSpells();
        }
        else if (spellChoice == 4)
        {
            // Casting uses the character's own known spells, not the global spell list.
            auto knownSpells = c.getSpellBook().getAllSpells();

            if (knownSpells.empty())
            {
                io.os() << "Character has no spells in their spellbook.\n";
                continue;
            }

            c.getSpellBook().displaySpellsWithIndex();
            io.os() << "0. Back\n";
            const int selectedSpell = io.readMenuChoice(
                "Select spell number: ", static_cast<int>(knownSpells.size()));
            if (selectedSpell == 0) continue;

            const Spell& spellToCast = knownSpells[selectedSpell - 1];
            const int spellLevel = spellToCast.getSpellLevel();

            if (spellLevel == 0)
            {
                io.os() << spellToCast.getSpellName() << " is a cantrip and does not use a spell slot.\n";
                continue;
            }

            // Allow upcasting, but never casting below the spell's base level.
            const int slotLevel = io.readInt(
                "Cast " + spellToCast.getSpellName() + " using what slot level? (" +
                std::to_string(spellLevel) + "-9): ", spellLevel, 9);

            if (c.getSpellSlots().useSlot(slotLevel))
            {
                io.os() << spellToCast.getSpellName() << " cast using a level "
                          << slotLevel << " slot.\n";
                io.os() << "Remaining level " << slotLevel << " slots: "
                          << c.getSpellSlots().getCurrentSlots(slotLevel) << "/"
                          << c.getSpellSlots().getMaxSlots(slotLevel) << "\n";
            }
            else
            {
                io.os() << "No level " << slotLevel << " spell slots remaining.\n";
            }
        }
        else if (spellChoice == 5)
        {
            int slotEditChoice = -1;

            do
            {
                io.os() << "\n=== Edit Spell Slots ===\n";
                c.getSpellSlots().displaySlots();
                io.os() << "1. Set max slots for a level\n";
                io.os() << "2. Set current slots for a level\n";
                io.os() << "0. Back\n";
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
                        io.os() << "Max slots updated for level " << slotLevel << ".\n";
                    }
                    else
                    {
                        // Cannot exceed the max already configured for this level.
                        const int currentSlots = io.readInt(
                            "New current slots: ", 0,
                            c.getSpellSlots().getMaxSlots(slotLevel));

                        c.getSpellSlots().setCurrentSlots(slotLevel, currentSlots);
                        io.os() << "Current slots updated for level " << slotLevel << ".\n";
                    }
                }
            } while (slotEditChoice != 0);
        }
    } while (spellChoice != 0);
}
