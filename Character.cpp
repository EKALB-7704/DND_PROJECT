#include "Character.h"
#include "stats.h"

Character::Character(Player_stats AS)
{
    Character_stats = AS;
}

void Character::getStats()
{
    Character_stats.displayAbilityScores();
}