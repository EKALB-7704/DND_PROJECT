#include "stats.h"

void Player_stats::setAbility(int AS, int Ability)
{
    if(AS <= 20 && AS > 0)
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
    else 
    cout << "Ability score must be between 1 and 20";
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
