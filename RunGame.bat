@echo off
cd /d "%~dp0"
if exist "bin\Debug\OpenGL-Fishing-Game.exe" (
    echo Starting the game...
    start "" "bin\Debug\OpenGL-Fishing-Game.exe"
) else if exist "bin\Release\OpenGL-Fishing-Game.exe" (
    echo Starting the game...
    start "" "bin\Release\OpenGL-Fishing-Game.exe"
) else (
    echo Executable not found! 
    echo Please build the project in Code::Blocks first before running.
    pause
)
