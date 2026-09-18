cmake_minimum_required(VERSION 3.18)

# No compiler is hardcoded here. CMake uses whatever compiler is on the PATH,
# or the one chosen by a preset (CMakePresets.json) or -DCMAKE_CXX_COMPILER.

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

# GCC 8.x ships std::filesystem as a separate library; GCC 9.1+ includes it automatically.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.1)
    target_link_libraries(dnd_lib PUBLIC stdc++fs)
endif()

# gcov instrumentation is a GCC/Clang feature; MSVC has no equivalent flags.
if(ENABLE_COVERAGE AND MSVC)
    message(FATAL_ERROR "ENABLE_COVERAGE needs GCC or Clang, not MSVC")
endif()

if(ENABLE_COVERAGE)
    target_compile_options(dnd_lib PUBLIC -fprofile-arcs -ftest-coverage)
    target_link_options(dnd_lib PUBLIC --coverage)
endif()

# Main game executable
add_executable(DND_PROJECT src/P_main.cpp)
target_link_libraries(DND_PROJECT PRIVATE dnd_lib)

# Statically link the MinGW runtime so the exe runs without MinGW DLLs on the PATH
if(MINGW)
    target_link_options(DND_PROJECT PRIVATE -static)
endif()

# Required for CTest to discover and register tests
enable_testing()

# Test suite
add_subdirectory(tests)
