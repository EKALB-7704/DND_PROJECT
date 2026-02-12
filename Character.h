#include <iostream>
#include <string>

#include "stats.h"


using namespace std;


class Character
{
    private:
    string Name; 
    int age;
    int weight;
    int level;

    Player_stats Character_stats;
    
    

    public:

    Character(Player_stats AS);

    void getStats();


};