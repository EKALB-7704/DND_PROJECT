cmake_minimum_required(VERSION 3.18)

# Point CMake at the MinGW compiler on Windows.
# Must be set BEFORE the project() call.
if(WIN32)
    set(CMAKE_CXX_COMPILER "C:/Program Files/CodeBlocks/MinGW/bin/g++.exe")
    set(CMAKE_C_COMPILER   "C:/Program Files/CodeBlocks/MinGW/bin/gcc.exe")
endif()
# On Linux CMake finds GCC automatically — no paths needed

project(DND_PROJECT CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_COVERAGE "Build with gcov coverage instrumentation" OFF)

# All game logic sources — excludes main so tests can link to the same library
set(LIB_SOURCES
    src/P_Character.cpp
    src/P_CharacterFeatures.cpp
    src/P_CharacterManager.cpp
    src/P_ManagerHelpers.cpp
    src/P_EditDetails.cpp
    src/P_EditHealth.cpp
    src/P_EditSpells.cpp
    src/P_Rest.cpp
    src/P_RollChecks.cpp
    src/P_ManageInventory.cpp
    src/P_ManageFeatures.cpp
    src/P_GlobalSpells.cpp
    src/P_DiceRoller.cpp
    src/P_Inventory.cpp
    src/P_Item.cpp
    src/P_Weapon.cpp
    src/P_Armor.cpp
    src/P_Gear.cpp
    src/P_Spell.cpp
    src/P_SpellBook.cpp
    src/P_SpellSlots.cpp
    src/P_Wallet.cpp
    src/P_Colours.cpp
    src/P_ConsoleIO.cpp
    src/P_Validate.cpp
    src/P_SaveFormat.cpp
)

# Static library that both the game exe and the test exe link against
add_library(dnd_lib STATIC ${LIB_SOURCES})
target_include_directories(dnd_lib PUBLIC include)

# MinGW GCC 8.x ships filesystem as a separate library; GCC 9+ includes it automatically.
if(WIN32)
    target_link_libraries(dnd_lib PUBLIC stdc++fs)
endif()

if(ENABLE_COVERAGE)
    target_compile_options(dnd_lib PUBLIC -fprofile-arcs -ftest-coverage)
    target_link_options(dnd_lib PUBLIC --coverage)
endif()

# Main game executable
add_executable(DND_PROJECT src/P_main.cpp)
target_link_libraries(DND_PROJECT PRIVATE dnd_lib)

# Required for CTest to discover and register tests
enable_testing()

# Test suite
add_subdirectory(tests)
