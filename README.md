# Mega Drive - Hello Pong - Boilerplate
This is a simple Pong game for the Sega Mega Drive, demonstrating basic graphics and input handling.
It serves as a boilerplate for developing games on the Mega Drive platform.

![screenshot](.github/screenshot.png "Screenshot of the game in action")

## Features
- Simple Pong game mechanics
- Basic graphics rendering
- Input handling for player controls
- Score tracking 

## Requirements
- Docker for building the project
- Sega Mega Drive emulator for testing

## Building the Project
To build the project, run the following command in the terminal:
```bash
docker run --rm -v $PWD:/m68k -t registry.gitlab.com/doragasu/docker-sgdk:v2.00
```

## Running the Game
After building, you can run the game using a Sega Mega Drive emulator with file `out/rom.bin`. I would suggest to rename it to `game.md` for clarity.

```bash
mv out/rom.bin game.md
```
