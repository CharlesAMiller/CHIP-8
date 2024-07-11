/** @file main.c
 * TODO:
 *  - Finish adding all op codes
 *  - Finish adding execution behavior
 *  - (Done) Separate into different files
 *  - Test that the code compiles to Arduino
 *  - Add functionality tests
 */

#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SFML/Graphics.h>

u_int8_t get_key_pressed();
void load_program(u_int8_t *program);
void draw_screen(u_int8_t *screen, sfImage *image);

int main(int argc, char *argv[])
{

    srand(time(0));

    // Setup
    peripherals peripherals;
    peripherals.get_key_pressed = &get_key_pressed;

    u_int8_t memory[RAM_SIZE];
    u_int8_t program_memory[PROGRAM_SIZE];
    load_program(program_memory);

    chip8_config config;
    config.peripherals = &peripherals;
    config.memory = memory;
    config.program = program_memory;

    chip8 cpu = init(&config);

    const sfVideoMode mode = {SCREEN_W * 2, SCREEN_H * 2, 32};
    sfRenderWindow *window = sfRenderWindow_create(mode, "CHIP-8", sfResize | sfClose, NULL);

    sfImage *image = sfImage_create(SCREEN_W, SCREEN_H);
    sfImage_createMaskFromColor(image, sfBlack, 0);

    if (!window)
        return EXIT_FAILURE;

    sfTexture* texture = sfTexture_createFromImage(image, NULL);
    sfSprite* sprite = sfSprite_create();
    sfSprite_setTexture(sprite, texture, sfTrue);

    sfEvent event;
    while (sfRenderWindow_isOpen(window))
    {
        run(&cpu);

        while (sfRenderWindow_pollEvent(window, &event))
        {
            if (event.type == sfEvtClosed)
                sfRenderWindow_close(window);
        }

        sfRenderWindow_clear(window, sfBlack);

        draw_screen(cpu.state.screen, image);

        // Update the texture
        sfTexture_updateFromImage(texture, image, 0, 0);

        sfRenderWindow_drawSprite(window, sprite, NULL);

        sfRenderWindow_display(window);
    }
}

void draw_screen(u_int8_t *screen, sfImage *image)
{
    u_int8_t x = 0; 
    u_int8_t y = 0;
    u_int8_t pixels;
    sfColor color;

    for (int i = 0; i < SCREEN_BYTES; i++) {

        if (i % H_OFFSET == 0) 
        {
            x = 0;
            y++;
        }

        pixels = screen[i]; 
        color = sfBlack;
        for (int j = 8; j > 0; j--, pixels <<= 1) 
        {
            sfImage_setPixel(image, x, y, color);
            color = ((pixels & 0b10000000) != 0) ? sfWhite : sfBlack;
            x++;
        }

    }
}

// Still a placeholder
void load_program(u_int8_t *program)
{
    // Set reg 3 to 2 
    program[0] = 0x63;
    program[1] = 0x02;
    // Location of string 
    program[2] = 0xF3;
    program[3] = 0x29;
    // Display 
    program[4] = 0xD1;
    program[5] = 0x15;
    program[6] = 0x12;
    program[7] = 0x00;
}

u_int8_t get_key_pressed()
{
    u_int8_t pressed_key = 1;
    // scanf("%c", &pressed_key);
    return pressed_key;
}