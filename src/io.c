/**
 * @file io.c
 * @brief This module implements the IO behavior for the desktop application
 */
#include "io.h"

sfKeyCode keyMap[16] = {
    [0] = sfKeyNum0,
    [1] = sfKeyNum1,
    [2] = sfKeyNum2,
    [3] = sfKeyNum3,
    [4] = sfKeyNum4,
    [5] = sfKeyNum5,
    [6] = sfKeyNum6,
    [7] = sfKeyNum7,
    [8] = sfKeyNum8,
    [9] = sfKeyNum9,
    [0xA] = sfKeyA,
    [0xB] = sfKeyB,
    [0xC] = sfKeyC,
    [0xD] = sfKeyD,
    [0xE] = sfKeyE,
    [0xF] = sfKeyF};

void load_program(char *file_name, u_int8_t *program)
{
    long lSize;
    FILE *file = fopen(file_name, "rb");

    // obtain file size:
    fseek(file, 0, SEEK_END);
    lSize = ftell(file);
    rewind(file);

    fread(program, 1, lSize, file);
}

u_int8_t rand_byte()
{
    u_int8_t r = rand() % 256;
    return r;
}

u_int8_t get_key_pressed()
{
    // Block
    while (1)
    {
        for (int i = 0; i < 0xF; i++)
            if (is_key_pressed(i))
                return i;
    }
}

u_int8_t is_key_pressed(u_int8_t key)
{
    sfBool isPresed = sfKeyboard_isKeyPressed(keyMap[key]);
    return (isPresed == sfTrue) ? 1 : 0;
}
