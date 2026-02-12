

#include <iostream>
#include <string>

#include "stats.h"
#include "Character.h"

using namespace std;


int main()
{

// Test for stats input and display functions 
Player_stats stat_1(15,14,13,12,10,8); // Test custom constructs using 5e standard array  
Character char_1(stat_1); // assign stat object to character object

stat_1.setAbility(15, 1); // Check for proper changing 

stat_1.setAbility(26, 2); // Check for improper changing  



char_1.getStats();  // Display all stats via the character objects












    cin.get();
}
