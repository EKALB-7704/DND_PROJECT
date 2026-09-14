#ifndef _SPELLBOOK_
#define _SPELLBOOK_

#include "H_spells.h"
#include <vector>

class SpellBook
{
private:
    std::vector<Spell> knownSpells;

public:
    void addSpell(const Spell& spell);
    void removeSpell(std::string name);
    bool updateSpell(size_t index, const Spell& spell);
    void displayAllSpells() const;
    void displaySpellsWithIndex() const;
    void saveSpellBook(const std::string& filename) const;
    void loadSpellBook(const std::string& filename);

    std::vector<Spell> getSpellsByLevel(int level) const;
    std::vector<Spell> getAllSpells() const;
};

#endif
