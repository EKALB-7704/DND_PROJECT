

#include <iostream>
#include <string>

#include "stats.h"

using namespace std;


int main()
{
    
    Player_stats player_1;

    player_1.setAbility(15, 1);
    cout << player_1.getAbility(1);
    cin.get();
}
