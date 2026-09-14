#include "H_Wallet.h"
#include "H_SaveFormat.h"
#include <string>
#include <vector>
#include "H_DndExceptions.h"
#include <iostream>


// Money is an important aspect of DND and comes in 5 denominations as per the 5e ruleset
// starting from the Highest:
// Platinum = 10 Gold
// Gold = 10 Silver or 5 Electrum pieces
// Silver = 10 Copper
// Copper is the lowest
// Electrum pieces are unusual as they dont follow the convention of the other denominations, intended to be halfway between gold and silver, and are often unused.
// However, some Dungeon masters like using them, so they are included for clarity.




Wallet::Wallet() : copper(0), silver(0), electrum(0), gold(0), platinum(0) {}

int Wallet::getCopper() const { return copper; }
int Wallet::getSilver() const { return silver; }
int Wallet::getElectrum() const { return electrum; }
int Wallet::getGold() const { return gold; }
int Wallet::getPlatinum() const { return platinum; }

void Wallet::adjustCopper(int amount)   { copper   += amount; }
void Wallet::adjustSilver(int amount)   { silver   += amount; }
void Wallet::adjustElectrum(int amount) { electrum += amount; }
void Wallet::adjustGold(int amount)     { gold     += amount; }
void Wallet::adjustPlatinum(int amount) { platinum += amount; }

void Wallet::display() const {
    std::cout << "  PP: " << platinum
              << "  GP: " << gold
              << "  EP: " << electrum
              << "  SP: " << silver
              << "  CP: " << copper << std::endl;
}

void Wallet::save(std::ofstream& file) const {
    SaveFormat::writeHeader(file, SaveFormat::kWalletTag);
    file << copper << " " << silver << " " << electrum << " " << gold << " " << platinum << "\n";
}

void Wallet::load(std::ifstream& file) {
    SaveFormat::readHeader(file, SaveFormat::kWalletTag, "wallet.txt");

    std::string line;
    std::vector<int> f;
    if (!SaveFormat::readLine(file, line) || !SaveFormat::parseInts(line, 5, f))
        throw LoadError("wallet data unreadable - file may be corrupt");

    // A negative purse is not a legal state; clamp rather than store it.
    copper   = f[0] < 0 ? 0 : f[0];
    silver   = f[1] < 0 ? 0 : f[1];
    electrum = f[2] < 0 ? 0 : f[2];
    gold     = f[3] < 0 ? 0 : f[3];
    platinum = f[4] < 0 ? 0 : f[4];
}
