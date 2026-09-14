#include "H_CharacterManager.h"
#include "H_DndExceptions.h"
#include <iostream>
#include "H_Weapon.h"
#include "H_Armor.h"
#include "H_Gear.h"

// Character editor: inventory, equipment and currency.

// Inventory
void CharacterManager::manageInventory(Character& c) {
    int choice;

    do {
        // Inventory menu
        io.os() << "\n=== Inventory ===\n";
        io.os() << "1. View\n";
        io.os() << "2. Add Item\n";
        io.os() << "3. Remove Item\n";
        io.os() << "4. Currency\n";
        io.os() << "5. Equip armor / shield\n";
        io.os() << "6. Unequip armor / shield\n";
        io.os() << "0. Back\n";
        choice = io.readMenuChoice("Choice: ", 6);

        if (choice == 1) {
            c.showInventory();
        }
        else if (choice == 2) // Create new item
        {
            io.os() << "Item type:\n1. Weapon\n2. Armor\n3. Gear\n";
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
            io.os() << "Item added.\n";
        }
        else if (choice == 3) // Remove item by index
        {
            if (c.getInventory().size() == 0) {
                io.os() << "Inventory is empty.\n";
            } else {
                c.showInventory();
                io.os() << "0. Back\n";
                const int index = io.readMenuChoice("Index to remove: ", c.getInventory().size());
                if (index > 0) c.removeItem(index);
            }
        }
        else if (choice == 5) // Equip armor or shield
        {
            if (c.getInventory().size() == 0) {
                io.os() << "Inventory is empty.\n";
            } else {
                c.showInventory();
                io.os() << "0. Cancel\n";
                const int idx = io.readMenuChoice("Enter item number to equip: ",
                                                  c.getInventory().size());
                if (idx > 0) {
                    c.equipArmor(idx);
                    c.equipShield(idx);
                    io.os() << "Equipped. AC is now " << c.getAC() << ".\n";
                }
            }
        }
        else if (choice == 6) // Unequip Armor or shield
        {
            io.os() << "1. Unequip armor\n2. Unequip shield\n0. Back\n";
            const int uChoice = io.readMenuChoice("Choice: ", 2);
            if (uChoice == 1) { c.unequipArmor();  io.os() << "Armor unequipped.\n"; }
            if (uChoice == 2) { c.unequipShield(); io.os() << "Shield unequipped.\n"; }
        }
        else if (choice == 4) // Manage currency
        {
            int currChoice;
            do {
                io.os() << "\n=== Currency ===\n";
                c.showCurrency();
                io.os() << "1. Add currency\n";
                io.os() << "2. Spend currency\n";
                io.os() << "0. Back\n";
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

                    io.os() << (currChoice == 1 ? "Currency added.\n" : "Currency spent.\n");
                }
            } while (currChoice != 0);
        }

    } while (choice != 0);
}

// Methods for managing features
