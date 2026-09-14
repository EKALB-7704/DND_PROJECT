#ifndef SPELL_H
#define SPELL_H
#include <fstream>
#include <string>
#include <iostream>


class Spell
{

    private:

    //Member variables 
    std::string Spell_Name;
    std::string Spell_Type;
    std::string Spell_Effect;
    int Spell_Level;
    std::string Spell_Time;
    std::string Spell_Range;
    std::string Spell_Components;
    std::string Spell_Duration;
    std::string Spell_Saving_Throw;
    std::string Spell_Description;


    public: 

    //Constructors
    Spell();
    Spell(std::string s_n, std::string s_t, std::string s_e, int s_l, std::string s_ti, std::string s_r, std::string s_c, std::string s_d, std::string s_s, std::string s_de);

    //Setters

    void setSpellName(std::string s_n);
    void setSpellType(std::string s_t);
    void setSpellEffect(std::string s_e);
    void setSpellLevel(int s_l);
    void setSpellTime(std::string s_t);
    void setSpellRange(std::string s_r);
    void setSpellComponents(std::string s_c);
    void setSpellDuration(std::string s_d);
    void setSpellSavingThrow(std::string s_s);
    void setSpellDescription(std::string s_d);

    void saveToFile(std::ofstream& outFile) const;
    void loadFromFile(std::ifstream& inFile);

    //Getters
    std::string getSpellName() const;
    std::string getSpellType() const;
    std::string getSpellEffect() const;
    int getSpellLevel() const;
    std::string getSpellTime() const;
    std::string getSpellRange() const;
    std::string getSpellComponents() const;
    std::string getSpellDuration() const;
    std::string getSpellSavingThrow() const;
    std::string getSpellDescription() const;

    //Display Spell Properties
    void DisplaySpellProperties() const;
    

};

#endif // SPELL_H
