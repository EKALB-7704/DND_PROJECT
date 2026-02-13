#include <iostream>
#include <string>

#include "stats.h"


using namespace std;


class Character
{
    private:
    string Name; 
    int Age;
    int Weight;
    int Level;

    Player_stats Character_stats;
    
    

    public:
    Character();
    Character(string name, int age, int weight,int level, Player_stats AS); // Set up character object with an assigned stats object

    // Setters and Getters
    void setCharName(string n);
    void setAge(int a);
    void setWeight(int w);
    void setLevel(int level);

    string getCharName();
    int getAge();
    int getWeight();
    int getLevel();

    void getDetails();
    void getStats();




};