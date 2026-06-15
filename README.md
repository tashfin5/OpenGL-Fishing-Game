# 3D OpenGL Fishing Game

A 3D interactive fishing game developed using C++ and OpenGL. Catch colorful fishes as they rotate on a dynamic game board using a controllable fishing rod.

## Features
* **3D Graphics:** Uses OpenGL for rendering a 3D environment with lighting, materials, and textures.
* **Interactive Gameplay:** Move the fishing rod and hook to time your catches perfectly as the fish open and close their mouths.
* **Score & Timer:** Tracks time taken to catch all fishes and calculates a final score based on performance.
* **Audio:** Background music support (WAV and MP3 via MCI).
* **Dynamic Camera:** Adjustable camera angle to get the best view of the action.

## Controls
* **Space:** Attempt to catch a fish
* **Arrow Keys (Up/Down/Left/Right):** Move the fishing rod across the board
* **W / S:** Move the hook up and down
* **A / D:** Rotate the camera angle
* **M:** Mute / Unmute background music
* **L:** Reload background music path
* **R:** Reset / Restart the game
* **ESC:** Exit game

## How to Run
1. **Quick Play:** Double-click `OpenGL-Fishing-Game.exe` in the root folder. The game comes with `freeglut.dll` and is statically linked, so no additional installations are required to run it!
2. **Build from Source:** Open the `OpenGL-Fishing-Game.cbp` file in **Code::Blocks** and build the project (Ctrl+F9). Make sure you have OpenGL and FreeGLUT properly set up in your compiler.
3. **Run Script:** You can also use the provided `RunGame.bat` script to quickly compile and run the game if you have MinGW installed.

## Project Structure
* `src/main.cpp`: Main game logic, rendering, and input handling.
* `textures/`: Contains BMP textures used for the game board and fishes.
* `audio/`: Contains the background music and the `music_path.txt` config file.
* `OpenGL-Fishing-Game.cbp`: Code::Blocks project configuration file.

## Requirements
* C++ Compiler (e.g., MinGW)
* OpenGL, GLU, and GLUT/FreeGLUT libraries
* Windows OS (for `winmm.lib` audio features)
