#include "H_SpellSlots.h"
#include "H_SaveFormat.h"
#include "H_DndExceptions.h"
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

void SpellSlots::setSlots(int level, int max)
{
    if (level < 1 || level > 9 || max < 0)
    {
        return;
    }

    // Treat changing the slot table as a full reset for that spell level.
    maxSlots[level] = max;
    currentSlots[level] = max;
}

void SpellSlots::setCurrentSlots(int level, int current)
{
    if (level < 1 || level > 9)
    {
        return;
    }

    int max = getMaxSlots(level);
    if (max == 0)
    {
        // A level with no configured slots cannot hold any current slots.
        currentSlots[level] = 0;
        return;
    }

    currentSlots[level] = std::clamp(current, 0, max);
}

bool SpellSlots::useSlot(int level)
{
    if (level < 1 || level > 9)
    {
        return false;
    }

    if (getCurrentSlots(level) > 0)
    {
        currentSlots[level]--;
        return true;
    }
    return false;
}

void SpellSlots::resetSlots()
{
    // A long-rest style reset restores each configured level to its max value.
    for (const auto& slot : maxSlots)
    {
        currentSlots[slot.first] = slot.second;
    }
}

int SpellSlots::getMaxSlots(int level) const
{
    auto it = maxSlots.find(level);
    return (it != maxSlots.end()) ? it->second : 0;
}

int SpellSlots::getCurrentSlots(int level) const
{
    auto it = currentSlots.find(level);
    return (it != currentSlots.end()) ? it->second : 0;
}

bool SpellSlots::hasAvailableSlot(int level) const
{
    return getCurrentSlots(level) > 0;
}

bool SpellSlots::hasAnySlots() const
{
    for (const auto& slot : maxSlots)
    {
        if (slot.second > 0)
        {
            return true;
        }
    }
    return false;
}

void SpellSlots::displaySlots() const
{
    if (maxSlots.empty())
    {
        std::cout << "No spell slots configured.\n";
        return;
    }

    for (const auto& slot : maxSlots)
    {
        int level = slot.first;
        std::cout << "Level " << level
                  << ": " << getCurrentSlots(level)
                  << "/" << slot.second << "\n";
    }
}

void SpellSlots::save(std::ofstream& file) const
{
    // Save both max and current counts so partially-spent slot pools persist.
    SaveFormat::writeHeader(file, SaveFormat::kSpellSlotsTag);
    file << maxSlots.size() << "\n";
    for (const auto& slot : maxSlots)
    {
        file << slot.first << " "
             << slot.second << " "
             << getCurrentSlots(slot.first) << "\n";
    }
}

void SpellSlots::load(std::ifstream& file)
{
    maxSlots.clear();
    currentSlots.clear();

    SaveFormat::readHeader(file, SaveFormat::kSpellSlotsTag, "spellslots.txt");

    const int count = SaveFormat::readCount(file, "spellslots.txt");

    std::string line;
    std::vector<int> f;
    for (int i = 0; i < count; i++)
    {
        if (!SaveFormat::readLine(file, line) || !SaveFormat::parseInts(line, 3, f))
            throw LoadError("spell slot entry unreadable in spellslots.txt");

        // Spell levels run 1-9; anything else is not a slot pool we can hold.
        if (f[0] < 1 || f[0] > 9) continue;
        const int max = f[1] < 0 ? 0 : f[1];
        const int current = f[2] < 0 ? 0 : (f[2] > max ? max : f[2]);

        // Load max first, then apply the saved current count for that level.
        setSlots(f[0], max);
        setCurrentSlots(f[0], current);
    }
}
