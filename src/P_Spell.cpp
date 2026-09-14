#include "H_Spell.h"

//-----------------------------------------------------//
// Constructors for creating empty spells or fully-filled spell entries.

Spell::Spell()
{
    Spell_Name = "";
    Spell_Type = "";
    Spell_Effect = "";
    Spell_Level = 0;
    Spell_Time = "";
    Spell_Range = "";
    Spell_Components = "";
    Spell_Duration = "";
    Spell_Saving_Throw = "";
    Spell_Description = "";
}

Spell::Spell(std::string s_n, std::string s_t, std::string s_e, int s_l, std::string s_ti, std::string s_r, std::string s_c, std::string s_d, std::string s_s, std::string s_de)
{
    Spell_Name = s_n;
    Spell_Type = s_t;
    Spell_Effect = s_e;
    Spell_Level = s_l;
    Spell_Time = s_ti;
    Spell_Range = s_r;
    Spell_Components = s_c;
    Spell_Duration = s_d;
    Spell_Saving_Throw = s_s;
    Spell_Description = s_de;
}
//-----------------------------------------------------//
// Setters update individual spell fields after creation.
void Spell::setSpellName(std::string s_n)
{
    Spell_Name = s_n;
}

void Spell::setSpellType(std::string s_t)
{
    Spell_Type = s_t;
}

void Spell::setSpellEffect(std::string s_e)
{
    Spell_Effect = s_e;
}

void Spell::setSpellLevel(int s_l)
{
    Spell_Level = s_l;
}

void Spell::setSpellTime(std::string s_ti)
{
    Spell_Time = s_ti;
}

void Spell::setSpellRange(std::string s_r)
{
    Spell_Range = s_r;
}

void Spell::setSpellComponents(std::string s_c)
{
    Spell_Components = s_c;
}

void Spell::setSpellDuration(std::string s_d)
{
    Spell_Duration = s_d;
}

void Spell::setSpellSavingThrow(std::string s_s)
{
    Spell_Saving_Throw = s_s;
}

void Spell::setSpellDescription(std::string s_de)
{
    Spell_Description = s_de;
}

//-----------------------------------------------------//
// Getters return the stored spell data for display and menu logic.

std::string Spell::getSpellName() const
{
    return Spell_Name;
}

std::string Spell::getSpellType() const
{
    return Spell_Type;
}

std::string Spell::getSpellEffect() const
{
    return Spell_Effect;
}

int Spell::getSpellLevel() const
{
    return (Spell_Level);
}

std::string Spell::getSpellTime() const
{
    return Spell_Time;
}

std::string Spell::getSpellRange() const 
{
    return Spell_Range;
}

std::string Spell::getSpellComponents() const
{
    return Spell_Components;
}

std::string Spell::getSpellDuration() const
{
    return Spell_Duration;
}

std::string Spell::getSpellSavingThrow() const
{
    return Spell_Saving_Throw;
}

std::string Spell::getSpellDescription() const
{
    return Spell_Description;
}

//-----------------------------------------------------//
// Print the full spell entry in a readable format.
void Spell::DisplaySpellProperties() const
{
    std::cout << "Spell Name: " << getSpellName() << std::endl;
    std::cout << "Spell Type: " << getSpellType() << std::endl;
    std::cout << "Spell Effect: " << getSpellEffect() << std::endl;
    std::cout << "Spell Level: " << getSpellLevel() << std::endl;
    std::cout << "Spell Time: " << getSpellTime() << std::endl;
    std::cout << "Spell Range: " << getSpellRange() << std::endl;
    std::cout << "Spell Components: " << getSpellComponents() << std::endl;
    std::cout << "Spell Duration: " << getSpellDuration() << std::endl;
    std::cout << "Spell Saving Throw: " << getSpellSavingThrow() << std::endl;
    std::cout << "Spell Description: " << getSpellDescription() << std::endl;
}
//-----------------------------------------------------//
// Write one spell entry to file in a fixed field order.
void Spell::saveToFile(std::ofstream& outFile) const
{
    outFile << Spell_Name << "\n";
    outFile << Spell_Type << "\n";
    outFile << Spell_Effect << "\n";
    outFile << Spell_Level << "\n";
    outFile << Spell_Time << "\n";
    outFile << Spell_Range << "\n";
    outFile << Spell_Components << "\n";
    outFile << Spell_Duration << "\n";
    outFile << Spell_Saving_Throw << "\n";
    outFile << Spell_Description << "\n";

    outFile << "END_SPELL\n"; // delimiter
}

//-----------------------------------------------------//
// Read one spell entry back from file using the same field order.
void Spell::loadFromFile(std::ifstream& inFile)
{
    getline(inFile, Spell_Name);
    getline(inFile, Spell_Type);
    getline(inFile, Spell_Effect);

    inFile >> Spell_Level;
    inFile.ignore(); // clear newline

    getline(inFile, Spell_Time);
    getline(inFile, Spell_Range);
    getline(inFile, Spell_Components);
    getline(inFile, Spell_Duration);
    getline(inFile, Spell_Saving_Throw);
    getline(inFile, Spell_Description);

    std::string endMarker;
    getline(inFile, endMarker); // should read END_SPELL
}
