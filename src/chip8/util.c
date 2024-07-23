#include "util.h"

void print_screen(uint8_t *screen)
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
    uint8_t pixel_mask = 0b10000000;
    for (int i = 0; i < SCREEN_BYTES; i++)
    {
        if (i % H_OFFSET == 0)
            printf("\n%d\t", i / H_OFFSET);

        uint8_t pixels = screen[i];
        for (int i = 8; i > 0; i--, pixels <<= 1)
            printf("%c", ((pixels & pixel_mask) != 0) ? SET_PIXEL : UNSET_PIXEL);
    }
}

void print_state(state *state) 
{
    printf("PC: %x I: %x ", state->PC, state->I);
    for (int i = 0; i < REGISTER_COUNT; i++) 
        printf(" V[%d]: %x", i, state->V[i]);
    printf(" SP: %x", state->SP);
    for (int i = 0; i < STACK_COUNT; i++)
        printf(" S[%d]: %x", state->stack[i]);
    printf("\n");
}