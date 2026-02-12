
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

    void setAbility(int AS, int Ability);
    int getAbility(int Ability);


};


#endif

