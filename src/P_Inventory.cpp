#include "H_Inventory.h"
#include "H_SaveFormat.h"
#include "H_DndExceptions.h"
#include <iostream>

void Inventory::addItem(std::unique_ptr<Item> item) {
    items.push_back(std::move(item));
}

void Inventory::removeItem(int index) {
    if (index - 1 >= 0 && index - 1 < static_cast<int>(items.size())) {
        items.erase(items.begin() + index - 1);
    }
}

void Inventory::display() const {
    if (items.empty()) {
        std::cout << "Inventory is empty.\n";
        return;
    }
    for (size_t i = 0; i < items.size(); i++) {
        std::cout << "\n[" << i + 1 << "] ";
        items[i]->display();
    }
}

int Inventory::size() const {
    return static_cast<int>(items.size());
}

const Item& Inventory::getItem(int index) const {
    return *items[index - 1];
}

void Inventory::save(std::ofstream& file) const {
    SaveFormat::writeHeader(file, SaveFormat::kInventoryTag);
    file << items.size() << "\n";
    for (const auto& item : items) {
        item->save(file);
    }
}

void Inventory::load(std::ifstream& file) {
    items.clear();
    SaveFormat::readHeader(file, SaveFormat::kInventoryTag, "inventory.txt");

    // Bounded: a corrupt count used to drive the item factory off the end
    // of the file.
    const int count = SaveFormat::readCount(file, "inventory.txt");
    for (int i = 0; i < count; i++) {
        items.push_back(Item::loadFromFile(file));
    }
}
