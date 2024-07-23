/**
 * @file io.h
 * @brief This module is used for the desktop application version of the CHIP-8 device
 *  It is used to capture input/output via a keyboard (provided by cSFML) and to load program files
 */
#ifndef IO_H
#define IO_H

#include <SFML/Graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * @brief A mapping of the CHIP-8 key codes to cSFML sfKeyCode values
 */
extern sfKeyCode keyMap[16];

/**
 * @brief A blocking function that waits for the user to press a key
 * This function adheres to the signature described by the peripheral in @see chip8.h
 *
 * @return The key code presed
 */
uint8_t get_key_pressed();

/**
 * @brief A function that checks if the given key code was pressed
 * This function adheres to the signature described by the peripheral in @see chip8.h
 * @param key - The key to be checked
 * @returns 1 - if the key was pressed. 0 otherwise.
 */
uint8_t is_key_pressed(uint8_t key);

/**
 * @brief A function that returns a random value between 0 - 255 (inclusive)
 * This function adheres to the signature described by the peripheral in @see chip8.h
 *
 * @returns a random number between 0 - 255 (inclusive)
 */
uint8_t rand_byte();

/**
 * @brief Loads the give file ands writes its contents to the given program memory
 *
 * @param file_name - The path of the program file to be read from
 * @param program - The pointer to the memory location for the program to be loaded to
 */
void load_program(char *file_name, uint8_t *program);

#endif