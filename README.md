# [CHIP-8](https://en.wikipedia.org/wiki/CHIP-8) Handheld

The goal of this project is to ultimately have a physical handheld device that runs pre-loaded games run on our CHIP-8 emulator. 

## Project Components

### Code 

### About
Housed under `/src`, the code is written in C with the intent of being run on Arduino or another similar embedded device.

#### Running 

Note: The desktop application has only been tested on MacOS at this time. 

Presently, the desktop application uses [cSFML](https://www.sfml-dev.org/download/csfml/) for rendering, audio, and input handling.

To run the desktop version of the application:
1. Install and properly link cSFML 2.6.1
2. Install Make (if not already installed)
3. From `/src` run `make` 
4. Run the application with a chip8 program as an argument `./chip8 program.ch8`

Optionally, you can create and run the test suite. This should not require any cSFML dependencies.
1. Install Make (if not already installed)
2. From `/src` run `make test_chip8`
3. Run the test suite `./test_chip8` to verify core functionality of the CHIP-8 implementation

Note: These tests are not exhaustive.

## Current Objectives
- Use [circuit.io](https://www.circuito.io/) to create breadboard a PoC of hardware necessary to run our CHIP-8
- Implement basic CHIP-8 in C