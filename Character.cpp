#include "Character.h"
#include "stats.h"

Character::Character()
{
    Name = "";
    Age = 0;
    Weight = 0;
    Level = 1;
}

Character::Character(string name, int age, int weight, int level, Player_stats AS)
{
    Name = name;
    Age = age;
    Weight = weight;
    Level = level;
    Character_stats = AS;
}

void Character::setCharName(string n)
{
    Name = n;
}

void Character::setAge(int a)
{
    Age = a;
}

void Character::setWeight(int w)
{
    Weight = w;
}

void Character::setLevel(int l)
{
    Level = l;
}

string Character::getCharName()
{
    return Name;
}

int Character::getAge()
{
    return Age;
}

int Character::getWeight()
{
    return Weight;
}

int Character::getLevel()
{
    return Level;
}

void Character::getDetails()
{
    cout << "Name:" << getCharName() << endl;
    cout << "Age: "<< getAge() << endl;
    cout << "Weight: " << getWeight() << endl;
    cout << "Level: " << getLevel() << endl;
}

void Character::getStats()

{
    Character_stats.displayAbilityScores();
}
