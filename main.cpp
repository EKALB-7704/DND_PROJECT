

#include <iostream>
#include <string>

#include "stats.h"

using namespace std;


int main()
{
   // Test for stats input and display functions 
    Player_stats player_1(15,14,13,12,10,8);

    player_1.setAbility(15, 1); // Check for proper changing and displaying of stats
    cout << player_1.getAbility(1) << endl;


    player_1.setAbility(26, 2); // Check for improper changing and displaying of stats 
    cout << player_1.getAbility(2) << endl;











    cin.get();
}
