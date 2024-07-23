#include "../chip8/chip8.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

void test_decode();
void test_clear_display(state *state);
void test_return(state *state);
void test_reg_dump(state *state);
void test_load_reg(state *state);
void test_math(state *state);
void test_bcd(state *state);

void clear_display_stub(uint8_t *screen);

int main(int argc, char *argv[])
{
    test_decode();

    uint8_t memory[RAM_SIZE];
    uint8_t program_memory[PROGRAM_SIZE];
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

void clear_display_stub(uint8_t *screen)
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
    // assert(state->stack[1] == 0);
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
        .type = REG_DUMP,
        .x = 0xF 
    };

    assert(state->I == 0x150);
    assert(state->memory[state->I] == 0);
    execute(&decoded_op, state, NULL);
    for (int i = 1; i < REGISTER_COUNT - 1; i++) 
        assert(state->memory[state-> I + i] == i);

    // Do a subset 
    for (int i = 0; i < REGISTER_COUNT; i++)
        state->V[i] = 2 * i;
    decoded_op.x = 1;
    execute(&decoded_op, state, NULL);
    assert(state->memory[state->I] == 0);
    assert(state->memory[state->I + 1] == 2);
    // Shouldn't haveen overwritten
    assert(state->memory[state->I + 2] == 2);
}

void test_load_reg(state *state)
{
    state->I = 0x150;

    for (int i = 0; i < REGISTER_COUNT; i++) 
        state->V[i] = 100;

    for (int i = 0; i < REGISTER_COUNT; i++) 
        state->memory[state->I + i] = i;

    op decoded_op = {
        .type = REG_LOAD,
        .x = 0xF
    };

    assert(state->I == 0x150);
    assert(state->memory[state->I] == 0);
    for (int i = 0; i < REGISTER_COUNT; i++)
        assert(state->V[i] == 100);
    execute(&decoded_op, state, NULL);
    for (int i = 0; i < REGISTER_COUNT; i++) 
        assert(state->V[i] == i);

    decoded_op.x = 1;
    // Do a subset
    for (int i = 0; i < REGISTER_COUNT; i++)
        state->V[i] = 0;

    execute(&decoded_op, state, NULL);
    assert(state->V[0] == 0);
    assert(state->V[1] == 1);
    assert(state->V[2] == 0);
    assert(state->V[3] == 0);
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

    // Shift left
    state->V[1] = 0xFF;
    decoded_op.type = SHIFT_LEFT;
    execute(&decoded_op, state, NULL);
    assert(state->V[1] == 0xFE);
    assert(state->V[0xF] == 1);
    // This time where the MSB will be 0
    state->V[1] = 0;
    execute(&decoded_op, state, NULL);
    assert(state->V[0xF] == 0);

    // Shift right
    state->V[1] = 1;
    decoded_op.type = SHIFT_RIGHT;
    execute(&decoded_op, state, NULL);
    assert(state->V[1] == 0);
    assert(state->V[0xF] == 1);
    execute(&decoded_op, state, NULL);
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

void test_decode()
{
    op op;
    uint8_t instruction[2] = { 0x00, 0xE0 };
    decode(instruction, &op);
    assert(op.type == CLEAR_DISPLAY);

    instruction[0] = 0x00; instruction[1] = 0xEE;
    decode(instruction, &op);
    assert(op.type == RET);

    instruction[0] = 0x12; instruction[1] = 0x34;
    decode(instruction, &op);
    assert(op.type == JUMP);
    assert(op.nnn == 0x234);

    instruction[0] = 0x22; instruction[1] = 0x34;
    decode(instruction, &op);
    assert(op.type == CALL);
    assert(op.nnn == 0x234);

    instruction[0] = 0x31; instruction[1] = 0x34;
    decode(instruction, &op);
    assert(op.type == IF_EQ);
    assert(op.nn == 0x34);
    assert(op.x == 1);

    instruction[0] = 0x41; instruction[1] = 0x34;
    decode(instruction, &op);
    assert(op.type == IF_NEQ);
    assert(op.nn == 0x34);
    assert(op.x == 1);

    instruction[0] = 0x51; instruction[1] = 0x30;
    decode(instruction, &op);
    assert(op.type == IF_EQ_REG);
    assert(op.y == 3);
    assert(op.x == 1);

    instruction[0] = 0x61; instruction[1] = 0xFF;
    decode(instruction, &op);
    assert(op.type == SET_REG);
    assert(op.nn == 0xFF);
    assert(op.x == 1);

    instruction[0] = 0x71; instruction[1] = 0xFF;
    decode(instruction, &op);
    assert(op.type == ADD_REG);
    assert(op.nn == 0xFF);
    assert(op.x == 1);
  
    instruction[0] = 0x81; instruction[1] = 0x30;
    decode(instruction, &op);
    assert(op.type == SET_REG_BY_REG);
    assert(op.y == 3);
    assert(op.x == 1);

    instruction[0] = 0x81; instruction[1] = 0x31;
    decode(instruction, &op);
    assert(op.type == OR);
    assert(op.y == 3);
    assert(op.x == 1);

    instruction[0] = 0x81; instruction[1] = 0x32;
    decode(instruction, &op);
    assert(op.type == AND);
    assert(op.y == 3);
    assert(op.x == 1);

    instruction[0] = 0x81; instruction[1] = 0x33;
    decode(instruction, &op);
    assert(op.type == XOR);
    assert(op.y == 3);
    assert(op.x == 1);

    instruction[0] = 0x81; instruction[1] = 0x34;
    decode(instruction, &op);
    assert(op.type == ADD_BY_REG);
    assert(op.y == 3);
    assert(op.x == 1);
    
    instruction[0] = 0x81; instruction[1] = 0x35;
    decode(instruction, &op);
    assert(op.type == SUB);
    assert(op.y == 3);
    assert(op.x == 1);
 
    instruction[0] = 0x81; instruction[1] = 0x36;
    decode(instruction, &op);
    assert(op.type == SHIFT_RIGHT);
    assert(op.x == 1);

    instruction[0] = 0x81; instruction[1] = 0x37;
    decode(instruction, &op);
    assert(op.type == SUBN);
    assert(op.y == 3);
    assert(op.x == 1);

    instruction[0] = 0x81; instruction[1] = 0x3E;
    decode(instruction, &op);
    assert(op.type == SHIFT_LEFT);
    assert(op.x == 1);

    instruction[0] = 0x91; instruction[1] = 0x30;
    decode(instruction, &op);
    assert(op.type == SKIP_NEQ);
    assert(op.x == 1);
    assert(op.y == 3);

    // Will also be read as SNEQ (not strict)
    instruction[0] = 0x91; instruction[1] = 0x3E;
    decode(instruction, &op);
    assert(op.type == SKIP_NEQ);
    assert(op.x == 1);
    assert(op.y == 3);

    instruction[0] = 0xA1; instruction[1] = 0x23;
    decode(instruction, &op);
    assert(op.type == SET_I_REG);
    assert(op.nnn == 0x123);

    instruction[0] = 0xB1; instruction[1] = 0x23;
    decode(instruction, &op);
    assert(op.type == BNNN);
    assert(op.nnn == 0x123);

    instruction[0] = 0xC1; instruction[1] = 0x23;
    decode(instruction, &op);
    assert(op.type == RANDOM);
    assert(op.x == 1);
    assert(op.nn == 0x23);

    instruction[0] = 0xD1; instruction[1] = 0x25;
    decode(instruction, &op);
    assert(op.type == DRAW_SPRITE);
    assert(op.x == 1);
    assert(op.y == 2);
    assert(op.n == 5);

    instruction[0] = 0xE1; instruction[1] = 0x9E;
    decode(instruction, &op);
    assert(op.type == SKIP_IF_KEY);
    assert(op.x == 1);

    instruction[0] = 0xE1; instruction[1] = 0xA1;
    decode(instruction, &op);
    assert(op.type == SKIP_IF_NKEY);
    assert(op.x == 1);

    instruction[0] = 0xF1; instruction[1] = 0x07;
    decode(instruction, &op);
    assert(op.type == GET_DELAY);
    assert(op.x == 1);

    instruction[0] = 0xF1; instruction[1] = 0x0A;
    decode(instruction, &op);
    assert(op.type == GET_KEY);
    assert(op.x == 1);

    instruction[0] = 0xF1; instruction[1] = 0x15;
    decode(instruction, &op);
    assert(op.type == SET_DELAY);
    assert(op.x == 1);

    instruction[0] = 0xF1; instruction[1] = 0x18;
    decode(instruction, &op);
    assert(op.type == SET_AUDIO);
    assert(op.x == 1);

    instruction[0] = 0xF1; instruction[1] = 0x1E;
    decode(instruction, &op);
    assert(op.type == ADVANCE_I);
    assert(op.x == 1);

    instruction[0] = 0xF1; instruction[1] = 0x29;
    decode(instruction, &op);
    assert(op.type == SET_I_HEX_SPRITE);
    assert(op.x == 1);

    instruction[0] = 0xF1; instruction[1] = 0x33;
    decode(instruction, &op);
    assert(op.type == BCD);
    assert(op.x == 1);

    instruction[0] = 0xF1; instruction[1] = 0x55;
    decode(instruction, &op);
    assert(op.type == REG_DUMP);
    assert(op.x == 1); 

    instruction[0] = 0xF1; instruction[1] = 0x65;
    decode(instruction, &op);
    assert(op.type == REG_LOAD);
    assert(op.x == 1); 

}