#include "H_CharacterManager.h"
#include "H_ManagerHelpers.h"
#include <algorithm>
#include <iostream>
#include <vector>

// Character editor: short and long rests.
//
// These used to sit inside the Spells submenu because they reset spell slots,
// but a rest also restores HP and hit dice, so it belongs at the character
// level rather than under one of the things it happens to touch.
void CharacterManager::restCharacter(Character& c)
{
    int restChoice = -1;

    do
    {
        io.os() << "\n=== Rest ===\n";
        io.os() << "HP: " << c.getCurrentHP() << "/" << c.getMaxHP()
                << "   Hit dice: " << c.getHitDiceNum() << "/" << c.getLevel()
                << c.getHitDice() << "\n";
        io.os() << "1. Long Rest\n";
        io.os() << "2. Short Rest\n";
        io.os() << "0. Back\n";
        restChoice = io.readMenuChoice("Choice: ", 2);

        if (restChoice == 1)
        {
            c.getSpellSlots().resetSlots();
            c.setCurrentHP(c.getMaxHP());
            c.recoverHitDice();
            io.os() << "Long rest complete. HP, spell slots, and hit dice restored.\n";
            io.os() << "Hit dice: " << c.getHitDiceNum() << "/" << c.getLevel()
                    << c.getHitDice() << "\n";
        }
        else if (restChoice == 2)
        {
            if (ManagerHelpers::isWarlockClass(c))
            {
                c.getSpellSlots().resetSlots();
                io.os() << "Short rest complete. Warlock spell slots restored to full.\n";
            }
            else
            {
                io.os() << "Short rest complete. Spell slots unchanged for "
                        << c.getClass() << ".\n";
            }

            // All classes can spend hit dice during a short rest.
            io.os() << "Hit dice available: " << c.getHitDiceNum()
                    << "/" << c.getLevel() << c.getHitDice() << "\n";
            if (c.getHitDiceNum() > 0 && c.getCurrentHP() < c.getMaxHP())
            {
                const int toSpend = io.readInt(
                    "Spend how many hit dice to recover HP? (0 = No): ",
                    0, c.getHitDiceNum());

                if (toSpend > 0)
                {
                    int hpGained = 0;
                    const int sides = c.getHitDieSides();
                    if (sides > 0)
                    {
                        // Each die heals its roll plus the CON modifier, and
                        // never less than 0 -- a low CON cannot make resting hurt.
                        const int conMod = Character::getAbilityModifier(c.getConstitution());
                        const std::vector<int> rolls = dice.rollDice(toSpend, sides);

                        io.os() << "Rolling " << toSpend << c.getHitDice() << ", "
                                << ManagerHelpers::signedValue(conMod) << " CON per die\n";
                        io.os() << "Rolls: ";
                        for (size_t i = 0; i < rolls.size(); i++)
                        {
                            io.os() << rolls[i] << (i + 1 < rolls.size() ? ", " : "\n");
                            hpGained += std::max(0, rolls[i] + conMod);
                        }
                        io.os() << "HP recovered: " << hpGained << "\n";
                    }
                    else
                    {
                        // The hit die is not a valid dN. Input and loading both
                        // reject that, so this is a safety net: ask for a total
                        // rolled at the table rather than roll a zero-sided die.
                        hpGained = io.readInt(
                            "Enter total HP recovered (roll " + std::to_string(toSpend) +
                            c.getHitDice() + " + CON modifier per die): ", 0, 100000);
                    }
                    if (hpGained > 0)
                    {
                        int newHp = c.getCurrentHP() + hpGained;
                        c.setCurrentHP(newHp > c.getMaxHP() ? c.getMaxHP() : newHp);
                    }
                    c.spendHitDice(toSpend);
                    io.os() << "HP: " << c.getCurrentHP() << "/" << c.getMaxHP()
                            << "  Hit dice remaining: " << c.getHitDiceNum() << "\n";
                }
            }
        }
    } while (restChoice != 0);
}
