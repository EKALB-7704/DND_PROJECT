#include "H_SpellBook.h"
#include "H_SaveFormat.h"
#include "H_DndExceptions.h"
#include <fstream>

// Add a spell to the current spell list.
void Spellbook::addSpell(const Spell& spell)
{
    knownSpells.push_back(spell);
}

// Remove the first spell whose name matches the requested one.
void Spellbook::removeSpell(std::string name)
{
    for (auto it = knownSpells.begin(); it != knownSpells.end(); ++it)
    {
        if (it->getSpellName() == name)
        {
            knownSpells.erase(it);
            return;
        }
    }
}

// Replace one stored spell without changing the rest of the spellbook.
bool Spellbook::updateSpell(size_t index, const Spell& spell)
{
    if (index >= knownSpells.size())
    {
        return false;
    }

    knownSpells[index] = spell;
    return true;
}

// Show every spell and all of its saved details.
void Spellbook::displayAllSpells() const
{
    for (const auto& spell : knownSpells)
    {
        spell.DisplaySpellProperties();
        std::cout << "------------------\n";
    }
}

// Show a numbered spell list for menu selection.
void Spellbook::displaySpellsWithIndex() const
{
    if (knownSpells.empty())
    {
        std::cout << "No spells available.\n";
        return;
    }

    for (size_t i = 0; i < knownSpells.size(); i++)
    {
        std::cout << i + 1 << ". " 
                  << knownSpells[i].getSpellName() 
                  << " (Level " << knownSpells[i].getSpellLevel() << ")\n";
    }
}

// Return only spells that match a chosen spell level.
std::vector<Spell> Spellbook::getSpellsByLevel(int level) const
{
    std::vector<Spell> result;

    for (const auto& spell : knownSpells)
    {
        if (spell.getSpellLevel() == level)
        {
            result.push_back(spell);
        }
    }

    return result;
}

// Return a copy of all stored spells.
std::vector<Spell> Spellbook::getAllSpells() const
{
    return knownSpells;
}

//-----------------------------------------------------//
// Save the full spellbook to disk.
void Spellbook::saveSpellbook(const std::string& filename) const
{
    std::ofstream outFile(filename);

    if (!outFile)
        throw SaveError("cannot open '" + filename + "' for writing");

    SaveFormat::writeHeader(outFile, SaveFormat::kSpellbookTag);
    outFile << knownSpells.size() << "\n";

    for (const auto& spell : knownSpells)
    {
        spell.saveToFile(outFile);
    }

    outFile.close();
}

//-----------------------------------------------------//
// Load the spellbook back from disk and replace the current list.
void Spellbook::loadSpellbook(const std::string& filename)
{
    std::ifstream inFile(filename);

    if (!inFile)
        throw LoadError("cannot open '" + filename + "' for reading");

    knownSpells.clear();

    SaveFormat::readHeader(inFile, SaveFormat::kSpellbookTag, filename);

    const int count = SaveFormat::readCount(inFile, filename);

    for (int i = 0; i < count; i++)
    {
        Spell spell;
        spell.loadFromFile(inFile);
        knownSpells.push_back(spell);
    }

    inFile.close();
}
