#include "stats.h"

void Player_stats::setAbility(int AS, int Ability)
{
    switch (Ability)
    {
    case 1:
        Strength = AS;
        break;

    case 2:
        Dexterity = AS;
        break;

    case 3:
        Constitution = AS;
        break;

    case 4:
        Inteligence = AS;
        break;

    case 5:
        Wisdom = AS;
        break;

    case 6:
        Charisma = AS;
        break;

    default:
        cout << "Not an ability";
    }
}

int Player_stats::getAbility(int Ability)
{
    switch (Ability)
    {
    case 1:
        return Strength;
        break;

    case 2:
        return Dexterity;
        break;

    case 3:
        return Constitution;
        break;

    case 4:
        return Inteligence;
        break;

    case 5:
        return Wisdom;
        break;

    case 6:
        return Charisma ;
        break;

    default:
        
        return 0;
    }
}
