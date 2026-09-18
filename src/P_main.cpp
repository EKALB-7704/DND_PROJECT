#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include "H_CharacterManager.h"
#include "H_DataPaths.h"
#include "H_CharacterFeatures.h"
#include "H_ConsoleIO.h"
#include "H_SpellBook.h"
#include "H_Colours.h"
#include "H_DndExceptions.h"

static void printUsage(const char* program) {
  std::cout << "Usage: " << program << " [--data-dir <path>]\n"
            << "  --data-dir <path>  folder holding SpellBook.txt and characters/\n"
            << "                     (or set DND_DATA_DIR)\n";
}

int main(int argc, char* argv[]) {
  std::filesystem::path dataDirArg;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--data-dir" && i + 1 < argc) {
      dataDirArg = argv[++i];
    } else if (arg == "-h" || arg == "--help") {
      printUsage(argv[0]);
      return 0;
    } else {
      std::cerr << "Unknown argument: " << arg << "\n";
      printUsage(argv[0]);
      return 2;
    }
  }

  // Single console I/O layer, shared by every menu. Reads are validated and
  // re-prompted here rather than at each `std::cin >>` site.
  ConsoleIO io;

  try {

    // Found from the executable's location, so the program no longer has to
    // be started from the project root.
    const char* envDataDir = std::getenv("DND_DATA_DIR");
    const std::filesystem::path dataDir = DataPaths::resolveDataDir(
        dataDirArg, envDataDir ? envDataDir : "",
        DataPaths::executableDir(), std::filesystem::current_path());
    std::cout << "Data folder: " << dataDir.string() << "\n";

    // Initialize character and colour manager objects
    CharacterManager manager(io, dataDir);
    ColourManager col_manager;

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
            // Rolls on the manager's roller, shared with the character editor.
            case 5: manager.rollDice(); break;
            case 0: break;
        }

    } while (choice != 0);
    col_manager.setColour(WHITE); // Set terminal colour back to white before closing program
    return 0;

  } catch (const EndOfInput&) {
    // stdin closed (Ctrl-D, or a piped script running out). Not an error:
    // restore the terminal colour and leave quietly.
    ColourManager().setColour(WHITE);
    std::cout << "\nInput closed. Exiting.\n";
    return 0;
  } catch (const std::exception& e) {
    ColourManager().setColour(WHITE);
    std::cerr << "\nFatal error: " << e.what() << "\n";
    return 1;
  } catch (...) {
    ColourManager().setColour(WHITE);
    std::cerr << "\nUnknown fatal error.\n";
    return 1;
  }
}
