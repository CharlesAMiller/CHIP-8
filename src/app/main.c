/**
 * @file main.c
 * @brief The entrypoint for the desktop application/implementation of the CHIP-8 device
 *
 * This module demonstrates a generic use case of our CHIP-8 module @see chip8.h
 * It leverages function pointers passed through peripherals to keep the device more portable
 */

#include "graphics.h"
#include "../chip8/chip8.h"
#include "audio.h"
#include "audio.h"
#include "io.h"

void init_peripherals(peripherals *peripherals);

int main(int argc, char *argv[])
{
    srand(time(0));

    // Setup
    peripherals peripherals;
    init_peripherals(&peripherals);
    uint8_t memory[RAM_SIZE];
    uint8_t program_memory[PROGRAM_SIZE];
    load_program(argv[1], program_memory);
    chip8_config config = {&peripherals, memory, program_memory};

    // Init chip8
    chip8 cpu = chip8_init(&config);

    // Start graphics loop
    init_screen(SCREEN_W * 8, SCREEN_H * 8, 8.0f);
    start_render_loop(&cpu, 60);
}

void init_peripherals(peripherals *peripherals)
{
    peripherals->display = &draw_screen;
    peripherals->get_key_pressed = &get_key_pressed;
    peripherals->is_key_pressed = &is_key_pressed;
    peripherals->random = &rand_byte;
    peripherals->noise = &noise;
}