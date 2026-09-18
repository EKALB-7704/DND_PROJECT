@echo off
rem Windows quick build: configures and builds with the "mingw" CMake preset,
rem then runs the game from the project root so data\ is found.
rem Set MINGW_BIN to use a MinGW other than the CodeBlocks one.

if not defined MINGW_BIN set "MINGW_BIN=C:\Program Files\CodeBlocks\MinGW\bin"
if exist "%MINGW_BIN%\g++.exe" set "PATH=%MINGW_BIN%;%PATH%"

where g++ >nul 2>nul || (
    echo g++ not found. Add MinGW's bin folder to PATH or set MINGW_BIN.
    pause
    exit /b 1
)

echo Building DND_PROJECT...
cmake --preset mingw && cmake --build --preset mingw --target DND_PROJECT
if errorlevel 1 (
    echo Build failed.
    pause
    exit /b 1
)

echo Build successful! Running program...
build\DND_PROJECT.exe
