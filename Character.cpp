#include "Character.h"
#include "stats.h"

Character::Character()
{
    Name = "";
    Age = 0;
    Weight = 0;
    Level = 1;
}

Character::Character(string name,string char_class,string race, int age, int weight, int level, Player_stats AS)
{
    Name = name;
    Class = char_class;
    Age = age;
    Weight = weight;
    Level = level;
    Character_stats = AS;
}

void Character::setCharName(string n)
{
    Name = n;
}

void Character::setCharClass(string c)
{
    Class = c;
}

void Character::setRace(string r)
{
    Race = r;
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

void Character::setStatObj(Player_stats stat)
{
    Character_stats = stat;
}

string Character::getCharName()
{
    return Name;
}

string Character::getCharClass()
{
    return Class;
}

string Character::getRace()
{
    return Race;
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

Player_stats Character::getStatObj()
{
    return Character_stats;
}

void Character::getDetails()
{
    cout << "Name: " << getCharName() << endl;
    cout << "Class: " << getCharClass() << endl;
    cout << "Race: " << getRace() << endl;
    cout << "Age: "<< getAge() << endl;
    cout << "Weight: " << getWeight() << endl;
    cout << "Level: " << getLevel() << endl;
}

void Character::getStats()

{
     
    Character_stats.displayAbilityScores();
}
