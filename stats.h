
#ifndef _STATS_
#define _STATS_



#include <iostream>
#include <string>

using namespace std;

class Player_stats
{
    private:
    int Strength, Dexterity, Constitution, Inteligence, Wisdom, Charisma, Initiative, Proficiency;
    string Ability_scores[6] = {"Strength: ", "Dexterity: ", "Constitution: ", "Intelligence: ", "Wisdom: ", "Charisma: "};



    public:
    Player_stats();
    Player_stats(int s, int d, int c, int i, int w, int ch, int in, int p);


    void setAbility(int AS, int Ability);
    void setProficiency(int p);
    void setInitiative(int in);


    int getAbility(int Ability);
    int getAbilityModifier(int a);
    int getProficiency();
    int getInitiative();



    void displayAbilityScores();

    


};


#endif

