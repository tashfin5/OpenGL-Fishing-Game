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
1. **Open the Project:** Open the `OpenGL-Fishing-Game.cbp` file in **Code::Blocks**.
2. **Build:** Build the project (Ctrl+F9). Make sure you have OpenGL and GLUT/FreeGLUT properly set up in your compiler.
3. **Play:** Run the game directly from Code::Blocks, or use the provided `RunGame.bat` script in the root folder to quickly launch the built executable.

## Project Structure
* `src/main.cpp`: Main game logic, rendering, and input handling.
* `textures/`: Contains BMP textures used for the game board and fishes.
* `audio/`: Contains the background music and the `music_path.txt` config file.
* `OpenGL-Fishing-Game.cbp`: Code::Blocks project configuration file.

## Requirements
* C++ Compiler (e.g., MinGW)
* OpenGL, GLU, and GLUT/FreeGLUT libraries
* Windows OS (for `winmm.lib` audio features)
