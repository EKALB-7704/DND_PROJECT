#include "stats.h"

Player_stats::Player_stats()
{
     Strength = 0;
    Dexterity = 0;
    Constitution = 0;
    Inteligence = 0;
    Wisdom = 0;
    Charisma = 0;
}

Player_stats::Player_stats(int s, int d, int c, int i, int w, int ch)
{
    Strength = s;
    Dexterity = d;
    Constitution = c;
    Inteligence = i;
    Wisdom = w;
    Charisma = ch;
}

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
    cout << "Ability score must be between 1 and 20" << endl;
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

int Player_stats::getAbilityModifier(int a)
{
    int Ab_mod = 0;
    int Ab_check = getAbility(a);
    if (Ab_check == 20)
    {
         Ab_mod = 5;
    }
    else if (Ab_check == 19 || Ab_check == 18)
    {
         Ab_mod = 4;
    }
    else if (Ab_check == 17 || Ab_check == 16)
    {
         Ab_mod = 3;
    }
    else if (Ab_check == 15 || Ab_check == 14)
    {
        Ab_mod = 2;
    }
    else if (Ab_check == 13 || Ab_check == 12)
    {
        Ab_mod = 1;
    }
    else if (Ab_check == 11 || Ab_check == 10)
    {
        Ab_mod = 0;
    }
    else if (Ab_check == 9 || Ab_check == 8)
    {
        Ab_mod = -1;
    }
    else if (Ab_check == 7 || Ab_check == 6)
    {
        Ab_mod = -2;
    }
    else if (Ab_check == 5 || Ab_check == 4)
    {
        Ab_mod = -3;
    }
    else if (Ab_check == 3 || Ab_check == 2)
    {
        Ab_mod = -2;
    }
    else if (Ab_check == 1)
    {
        Ab_mod = -1;
    }
    return Ab_mod;



}

void Player_stats::displayAbilityScores()
{
    for (int i = 1; i <=6; i++ )
    {
        
        cout << Ability_scores[i - 1] << getAbility(i) << "(" << getAbilityModifier(i) << ")" << endl;
    }
}
