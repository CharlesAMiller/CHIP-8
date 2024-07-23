/**
 * @file graphics.h
 * @brief This module is used for the desktop application version of the CHIP-8 device
 *  It is used to display the screen buffer to a an application window via cSFML
 */
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SFML/Graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include "../chip8/chip8.h"

/**
 * @brief Image that is displayed to the screen
 */
extern sfImage *image;
/**
 * @brief The texture that wraps the above image
 */
extern sfTexture *texture;
/**
 * @brief The sprite that uses the texture to display content to the screen
 */
extern sfSprite *sprite;
/**
 * @brief The window responsible for visually displaying our device
 */
extern sfWindow *window;

/**
 * @brief Creates the cSFML window according to the given parameters
 * @param width - The width of the window to be created
 * @param heigh - The height of the window to be created
 * @param scale_factor - How the image should be scaled relative to the window size
 */
int init_screen(int width, int height, float scale_factor);

/**
 * @brief A function that draws the given CHIP-8 screen buffer to a desktop window
 * This function adheres to the signature described by the peripheral in @see chip8.h
 *
 * @param screen - The CHIP-8 screen buffer
 */
void draw_screen(uint8_t *screen);

/**
 * @brief Starts a loop that runs the given cpu, periodically rendering the contents
 * of the device's screen to the application window
 *
 * @param cpu - The CHIP-8 device to emulate and render for
 * @param frame_limit - Sets the rate at which the window is polled
 */
void start_render_loop(chip8 *cpu, unsigned int frame_limit);

#endif