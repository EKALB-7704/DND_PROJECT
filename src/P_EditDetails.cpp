#include "H_CharacterManager.h"
#include "H_DndExceptions.h"
#include <iostream>

// Character editor: the "character details" and "ability scores" sections.

// Character details -- name, race, class, level, speed and so on.
void CharacterManager::editDetails(Character& c)
{
    int char_edit_choice;
    io.os() << "What would you like to change? " << std::endl;
    for (int i = 0; i < 9; i++)
    {
        io.os() << i + 1 << "." << EditCharDetailsArray[i] << std::endl;
    }
    char_edit_choice = io.readMenuChoice("Choice: ", 9);
    switch (char_edit_choice)
    {

    case 1: //Name
        {
            std::string new_name = io.readName("Enter new Name: ");
            c.setName(new_name);
            break;
        }

    case 2: //Race
        {
            std::string new_race = io.readText("Enter new Race: ");
            c.setRace(new_race);
            break;
        }
          
    case 3: //Class
        {
            std::string new_class = io.readText("Enter new Class: ");
            c.setClass(new_class);
            break;
        }
    
    case 4: //Background
        {
            std::string new_background = io.readText("Enter new Background: ");
            c.setBackground(new_background);
            break;
            
        }
    case 5: //Alignment
        {
            std::string new_alignment = io.readText("Enter new Alignment: ");
            c.setAlignment(new_alignment);
            io.os() << "Alignment set" << std::endl;
            break;
        }
    case 6: //Age
        {
            int new_age = io.readInt("Enter new Age: ", 0, 100000);
            c.setAge(new_age);
            io.os() << "New age set" << std::endl;
            break;
        }

    case 7: //Weight
        {
            int new_weight = io.readInt("Enter new Weight: ", 0, 100000);
            c.setWeight(new_weight);
            io.os() << "New weight set" << std::endl;
            break;
        }

    case 8: //Level
        {
            c.setLevel(io.readInt("Enter new Level (1-20): ", 1, 20));
            break;
        }

    case 9: //Speed
        {
            int new_speed = io.readInt("Enter new Speed (ft): ", 0, 1000);
            c.setSpeed(new_speed);
            io.os() << "Speed set to " << new_speed << " ft\n";
            break;
        }

    }
}

// HP, hit dice, death saves and conditions.

// The six ability scores.
void CharacterManager::editAbilityScores(Character& c)
{
    io.os() << "What stat would you like to change? "  << std::endl;
    for (int i = 0; i < 6; i++)
    {
        io.os() << i + 1 << ". " << Ability_scores[i] << std::endl;
    }
    // abil was previously unvalidated, so a value outside 1-6 indexed
    // Ability_scores out of bounds.
    const int abil = io.readInt("Choice: ", 1, 6);
    const int val  = io.readInt("New " + Ability_scores[abil - 1], 1, 20);
    c.setStats(val, abil);
}

// Character spellbook, spell slots, and short/long rests.
