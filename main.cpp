

#include <iostream>
#include <string>

#include "stats.h"
#include "Character.h"
#include "equipment.h"

using namespace std;


int main()
{

// Test for stats input and display functions 
Player_stats stat_1(15,14,13,12,10,8,5,8); // Test custom constructs using 5e standard array  
Character char_1("Paul","Warlock","Human",15,0,1, stat_1); // assign stat object to character object
equipment weapon_1("","");

// Test setters and getters for character details
char_1.setCharName("Thrikna");
char_1.setCharClass("Fighter");
char_1.setRace("Orc");
char_1.setAge(20);
char_1.setWeight(80);
char_1.setLevel(5);

// Test getters and setters for stat details using character object
char_1.getStatObj().setInitiative(2);
char_1.getStatObj().setProficiency(3);
char_1.getStatObj().setAbility(18, 1); // Check for proper changing
char_1.getStatObj().setAbility(26, 2); // Check for improper changing 

// Test getters and setters for equipment objects
weapon_1.setEquipmentName("Greatsword");
weapon_1.setEquipmentUse("2d6 + 4 slash damage");


// Display all details and check for changes 
char_1.getDetails();
char_1.getStatObj().displayAbilityScores(); // Display all stats via the character objects

weapon_1.displayEquipmentDetails();














    cin.get();
}
