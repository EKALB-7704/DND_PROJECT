

#include <iostream>
#include <string>

#include "stats.h"
#include "Character.h"

using namespace std;


int main()
{

// Test for stats input and display functions 
Player_stats player_1(15,14,13,12,10,8); // Test custom constructs using 5e standard array  
Character char_1(player_1);

player_1.setAbility(15, 1); // Check for proper changing 

player_1.setAbility(26, 2); // Check for improper changing  

//player_1.displayAbilityScores(); // Display all stats

char_1.getStats();












    cin.get();
}
