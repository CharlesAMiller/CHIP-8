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


u_int8_t get_key_pressed();
void load_program(u_int8_t *program);

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

    while(1) 
    {
        run(&cpu);
    }
}

// Still a placeholder
void load_program(u_int8_t *program)
{
    // Set reg 1 to 123
    program[0] = 0x61;
    program[1] = 0x7B;
    // Set I to 0x200 + something (write after program)
    program[2] = 0xA2;
    program[3] = 0x06;
    // BCD
    program[4] = 0xF1;
    program[5] = 0x33;

    program[10] = 0xF1;
    program[11] = 0x0A;
}

u_int8_t get_key_pressed()
{
    u_int8_t pressed_key;
    scanf("%c", &pressed_key);
    return pressed_key;
}