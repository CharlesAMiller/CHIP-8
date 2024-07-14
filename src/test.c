#include "chip8.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

void test_clear_display(state *state);
void test_return(state *state);
void test_reg_dump(state *state);
void test_load_reg(state *state);
void test_math(state *state);
void test_bcd(state *state);

void clear_display_stub(u_int8_t *screen);

int main(int argc, char *argv[])
{
    u_int8_t memory[RAM_SIZE];
    u_int8_t program_memory[PROGRAM_SIZE];
    state test_state;
    init_state(&test_state, memory, program_memory);

    test_clear_display(NULL);
    test_return(NULL);
    test_math(NULL);
    test_reg_dump(&test_state);
    init_state(&test_state, memory, program_memory);
    test_load_reg(&test_state);
    init_state(&test_state, memory, program_memory);
    test_bcd(&test_state);
}

void clear_display_stub(u_int8_t *screen)
{
    printf("Printed screen");
}

void test_clear_display(state *state)
{
    if (state == NULL)
    {
        state = malloc(sizeof(state));
        memset(state->screen, 1, SCREEN_BYTES); 
    }

    op decoded_op = {
        .type = CLEAR_DISPLAY
    };

    peripherals peripherals = {
        .display = &clear_display_stub
    };

    assert(state->screen[0] == 1);
    execute(&decoded_op, state, &peripherals);
    assert(state->screen[0] == 0);
}

void test_return(state *state)
{
    if (state == NULL)
    {
        state = malloc(sizeof(state));
        state->stack[0] = 0xAAAA;
        state->stack[1] = 0xBBBB;
        state->SP = 1;
        state->PC = 0;
    }

    op decoded_op = {
        .type = RET
    };

    assert(state->PC == 0);
    assert(state->SP == 1);
    execute(&decoded_op, state, NULL);
    assert(state->PC == 0xBBBB);
    assert(state->SP == 0);
    assert(state->stack[1] == 0);
    execute(&decoded_op, state, NULL);
    assert(state->PC == 0xAAAA);
    assert(state->SP == 0);
}

void test_reg_dump(state *state)
{
    for (int i = 0; i < REGISTER_COUNT; i++) 
        state->V[i] = i;
    state->I = 0x150;

    op decoded_op = {
        .type = REG_DUMP
    };

    assert(state->I == 0x150);
    assert(state->memory[state->I] == 0);
    execute(&decoded_op, state, NULL);
    for (int i = 0; i < REGISTER_COUNT; i++) 
        assert(state->memory[state-> I + i] == i);
}

void test_load_reg(state *state)
{
    state->I = 0x150;

    for (int i = 0; i < REGISTER_COUNT; i++) 
        state->V[i] = 100;

    for (int i = 0; i < REGISTER_COUNT; i++) 
        state->memory[state->I + i] = i;

    op decoded_op = {
        .type = REG_LOAD 
    };

    assert(state->I == 0x150);
    assert(state->memory[state->I] == 0);
    assert(state->V[0] == 100);
    execute(&decoded_op, state, NULL);
    for (int i = 0; i < REGISTER_COUNT; i++) 
        assert(state->V[i] == i);
}

void test_math(state *state) 
{
    if (state == NULL) 
    {
        state = malloc(sizeof(state));
    }

    // ADD
    state->V[1] = 1;
    state->V[2] = 1;
    op decoded_op = {
        .x = 1,
        .y = 2,
        .type = ADD_BY_REG
    };
    assert(state->V[1] == 1);
    assert(state->V[2] == 1);
    execute(&decoded_op, state, NULL);
    assert(state->V[1] == 2);
    assert(state->V[2] == 1);
    assert(state->V[0xF] == 0);
    // ADD w Carry
    state->V[1] = 0xff;
    state->V[2] = 0x01;
    execute(&decoded_op, state, NULL);
    assert(state->V[1] == 0);
    assert(state->V[2] == 1);
    assert(state->V[0xF] == 1);

    // Sub 
    state->V[1] = 2;
    state->V[2] = 1;
    decoded_op.type = SUB;
    assert(state->V[1] == 2);
    assert(state->V[2] == 1);
    execute(&decoded_op, state, NULL);
    assert(state->V[1] == 1);
    assert(state->V[2] == 1);
    assert(state->V[0xF] == 1);
    // sub w borrow 
    state->V[1] = 0x00;
    state->V[2] = 0x01;
    execute(&decoded_op, state, NULL);
    assert(state->V[1] == 0xFF);
    assert(state->V[2] == 1);
    assert(state->V[0xF] == 0);

}

void test_bcd(state *state)
{
    state->I = 0x300;
    state->V[1] = 111;

    op decoded_op = {
        .x = 1,
        .type = BCD 
    };

    for (int i = 0; i < 3; i++)
        assert(state->memory[state->I + i] == 0);
    execute(&decoded_op, state, NULL);
    for (int i = 0; i < 3; i++)
        assert(state->memory[state->I + i] == 1);

    state->V[1] = 254;
    execute(&decoded_op, state, NULL);
    assert(state->memory[state->I] == 2);
    assert(state->memory[state->I + 1] == 5);
    assert(state->memory[state->I + 2] == 4);
}