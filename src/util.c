#include "util.h"
#include "chip8.h"

void print_screen(u_int8_t *screen)
{
    printf("\t");
    for (int i = 0; i < SCREEN_W; i++)
        if (i > 10)
            printf("%d", i / 10);
        else
            printf(" ");
    printf("\n\t");
    for (int i = 0; i < SCREEN_W; i++)
        printf("%d", i % 10);
    printf("\n");
    u_int8_t pixel_mask = 0b10000000;
    for (int i = 0; i < SCREEN_BYTES; i++)
    {
        if (i % H_OFFSET == 0)
            printf("\n%d\t", i / H_OFFSET);

        u_int8_t pixels = screen[i];
        for (int i = 8; i > 0; i--, pixels <<= 1)
            printf("%c", ((pixels & pixel_mask) != 0) ? SET_PIXEL : UNSET_PIXEL);
    }
}