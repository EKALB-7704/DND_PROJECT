#ifndef INVENTORY_H
#define INVENTORY_H

#include <vector>
#include <memory>
#include "H_Item.h"

class Inventory {
private:
    std::vector<std::unique_ptr<Item>> items;

public:
    // Move-only, since items are uniquely owned. Declaring this makes Character
    // non-copyable too, so std::vector<Character> must move on reallocation.
    // Without it MSVC tries to copy: its std::map (in SpellSlots) has a
    // non-noexcept move, which makes Character's move non-noexcept as well.
    Inventory() = default;
    Inventory(const Inventory&) = delete;
    Inventory& operator=(const Inventory&) = delete;
    Inventory(Inventory&&) noexcept = default;
    Inventory& operator=(Inventory&&) noexcept = default;

    void addItem(std::unique_ptr<Item> item);
    void removeItem(int index);
    void display() const;

    int size() const;
    const Item& getItem(int index) const;

    void save(std::ofstream& file) const;
    void load(std::ifstream& file);
};

#endif
