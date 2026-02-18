#ifndef _CHARACTER_
#define _CHARACTER_



#include <iostream>
#include <string>

#include "stats.h"


using namespace std;


class Character
{
    private:

    // Member variables
    string Name;
    string Class;
    string Race; 
    int Age;
    int Weight;
    int Level;

    Player_stats Character_stats;
    
    

    public:

    // Constructors
    Character();
    Character(string name, string char_class,string race, int age, int weight,int level, Player_stats AS); // Set up character object with an assigned stats object

    // Setters 
    void setCharName(string n);
    void setCharClass(string c);
    void setRace(string r);
    void setAge(int a);
    void setWeight(int w);
    void setLevel(int level);
    void setStatObj(Player_stats stat);

    // Getters
    string getCharName();
    string getCharClass();
    string getRace();
    int getAge();
    int getWeight();
    int getLevel();
    Player_stats& getStatObj();
    void getDetails();
    void getStats();




};

#endif