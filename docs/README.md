This is the READ ME for the DND character tracker

The purpose of this project is to create a program to track a persons dnd characters, saving the use of seperated files or character sheets.

if possible, we will expand of the scope of this project to allow dungeon masters to manage characters (both player characters and NPCs) for their campaigns.


this project aims to replicate the functionality of a dnd character sheet (Stats, abilities equipment, resources etc.) while adding the efficiency of a computer program.


Running Test build.

LINUX 
From the project root:

mkdir -p build (on first run only)
cd build
cmake ..
cmake --build .
ctest --output-on-failure


WINDOWS
From the project root (Command Prompt)
mkdir build (on first run only)
cd build
cmake -G "MinGW Makefiles" ..
cmake --build .
ctest --output-on-failure



