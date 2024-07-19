# [CHIP-8](https://en.wikipedia.org/wiki/CHIP-8) Handheld

The goal of this project is to ultimately have a physical handheld device that runs pre-loaded games run on our CHIP-8 emulator. 

## Project Components

### Code 
Housed under `/src`, the code is written in C. with the intent of being run on Arduino or another similar embedded device.

    .
    ├── ...
    ├── src                     # Top level of the project's code 
    │   ├── app                 # Code particular to running the desktop application implementing our CHIP-8 library
    │   ├── chip8               # Library code for our CHIP-8 implementation 
    │   └── test                # Test suite for testing functionality and behaviors of our CHIP-8 library 
    ├── Makefile                # The Makefile for building our app and test suite 
    └── ...


#### Desktop App 
![Desktop Application](docs/app.png)
Note: The desktop application has only been tested on MacOS at this time. 

Presently, the desktop application uses [cSFML](https://www.sfml-dev.org/download/csfml/) for rendering, audio, and input handling.

To run the desktop version of the application:
1. Install and properly link cSFML 2.6.1
2. Install Make (if not already installed)
3. Run `make` 
4. Run the application with a chip8 program as an argument `./chip8 program.ch8` (you can use `roms/test/3-corax+.ch8` as a basis)

#### Test Suite
This is an application that runs a series of tests against the functionality of our CHIP-8 implementation

To build and run the test suite:
1. Install Make (if not already installed)
2. Run `make test_chip8`
3. Run the test suite `./test_chip8` to verify core functionality of the CHIP-8 implementation

Note: These tests are not exhaustive.

## Current Objectives
- Use [circuit.io](https://www.circuito.io/) to create breadboard a PoC of hardware necessary to run our CHIP-8
- Implement basic CHIP-8 in C