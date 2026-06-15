@echo off
cd /d "%~dp0"
if exist "bin\Debug\OpenGL-Fishing-Game.exe" (
    echo Starting the game...
    start "" "bin\Debug\OpenGL-Fishing-Game.exe"
) else if exist "bin\Release\OpenGL-Fishing-Game.exe" (
    echo Starting the game...
    start "" "bin\Release\OpenGL-Fishing-Game.exe"
) else (
    echo Executable not found! Opening the Code::Blocks project...
    echo Please build the project (Ctrl+F9) and run it.
    start "" "OpenGL-Fishing-Game.cbp"
)
