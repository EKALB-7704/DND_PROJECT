
#ifndef _STATS_
#define _STATS_



#include <iostream>
#include <string>

using namespace std;

class Player_stats
{
    private:
    int Strength, Dexterity, Constitution, Inteligence, Wisdom, Charisma;



    public:
    Player_stats();
    Player_stats(int s, int d, int c, int i, int w, int ch);
    void setAbility(int AS, int Ability);
    int getAbility(int Ability);
    void displayAbilityScores();

    // NOTE make function to cycle through string array with stats to denote what each stat is!!!!!


};


#endif

