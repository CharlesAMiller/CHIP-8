/**
 * @file io.c
 * @brief This module implements the IO behavior for the desktop application
 */
#include "io.h"

sfKeyCode keyMap[16] = {
    [0] = sfKeyX,
    [1] = sfKeyNum1,
    [2] = sfKeyNum2,
    [3] = sfKeyNum3,
    [4] = sfKeyQ,
    [5] = sfKeyW,
    [6] = sfKeyE,
    [7] = sfKeyA,
    [8] = sfKeyS,
    [9] = sfKeyD,
    [0xA] = sfKeyZ,
    [0xB] = sfKeyC,
    [0xC] = sfKeyNum4,
    [0xD] = sfKeyR,
    [0xE] = sfKeyF,
    [0xF] = sfKeyV};

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
    sfBool isPressed = sfKeyboard_isKeyPressed(keyMap[key]);
    return (isPressed == sfTrue) ? 1 : 0;
}
