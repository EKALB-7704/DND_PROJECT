#include <iostream>
#include "H_CharacterManager.h"
#include "H_CharacterFeatures.h"
#include "H_ConsoleIO.h"
#include "H_DiceRoller.h"
#include "H_SpellBook.h"
#include "H_Colours.h"
#include "H_DndExceptions.h"

int main() {
  // Single console I/O layer, shared by every menu. Reads are validated and
  // re-prompted here rather than at each `std::cin >>` site.
  ConsoleIO io;

  try {

    // Initialize character and colour manager objects
    CharacterManager manager(io);
    Colour_manager col_manager;
    // Dice roller is available from the main menu as a utility tool.
    DiceRoller diceRoller;

    int choice;

    do {

        // Main menu
        std::cout << "\n=== DnD Manager ===\n";
        std::cout << "1. Create Character\n";
        std::cout << "2. View Characters\n";
        std::cout << "3. Manage Global Spells\n";
        std::cout << "4. Change Text Colour\n";
        std::cout << "5. Roll Dice\n";
        std::cout << "0. Exit\n";

        choice = io.readMenuChoice("Choice: ", 5);

        switch (choice) {
            case 1: manager.createCharacter(); break;
            case 2:
            {
                int characterMenuChoice = -1;
                do
                {
                    std::cout << "\n=== View Character ===\n";
                    std::cout << "1. Load Characters\n";
                    std::cout << "2. View Character Details\n";
                    std::cout << "3. Edit Character\n";
                    std::cout << "4. Save Characters\n";
                    std::cout << "0. Back\n";

                    characterMenuChoice = io.readMenuChoice("Choice: ", 4);

                    switch (characterMenuChoice)
                    {
                        case 1:
                            manager.loadAll();
                            break;
                        case 2:
                            manager.viewCharacters();
                            break;
                        case 3:
                            manager.editCharacter();
                            break;
                        case 4:
                            manager.saveAll();
                            break;
                        case 0:
                            break;
                    }
                } while (characterMenuChoice != 0);
                break;
            }
            case 3: manager.manageGlobalSpells(); break;
            case 4: col_manager.ChangeColour(io); break;
            case 5: diceRoller.promptAndRoll(io); break;
            case 0: break;
        }

    } while (choice != 0);
    col_manager.setColour(WHITE); // Set terminal colour back to white before closing program
    return 0;

  } catch (const EndOfInput&) {
    // stdin closed (Ctrl-D, or a piped script running out). Not an error:
    // restore the terminal colour and leave quietly.
    Colour_manager().setColour(WHITE);
    std::cout << "\nInput closed. Exiting.\n";
    return 0;
  } catch (const std::exception& e) {
    Colour_manager().setColour(WHITE);
    std::cerr << "\nFatal error: " << e.what() << "\n";
    return 1;
  } catch (...) {
    Colour_manager().setColour(WHITE);
    std::cerr << "\nUnknown fatal error.\n";
    return 1;
  }
}
