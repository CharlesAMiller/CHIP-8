/** @file chip8.c
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "chip8.h"

enum op_type op_type_lookup[0xE] = {
    [1] = JUMP,
    [2] = CALL,
    [3] = IF_EQ,
    [4] = IF_NEQ,
    [5] = IF_EQ_REG,
    [6] = SET_REG,
    [7] = ADD_REG,
    [9] = SKIP_NEQ,
    [0xA] = SET_I_REG,
    [0xB] = BNNN,
    [0xC] = RANDOM,
    [0xD] = DISPLAY};

enum op_type bit_op_type_lookup[9] = {
    [0] = SET_REG_BY_REG,
    [1] = OR,
    [2] = AND,
    [3] = XOR,
    [4] = ADD_BY_REG,
    [5] = SUB,
    [6] = SHIFT_RIGHT,
    [7] = SUBN,
    [8] = SHIFT_LEFT};

u_int8_t digit_sprites_data[0x50] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80, // F
};

void run(chip8 *cpu)
{
    static op decoded_instruction;
    static u_int8_t instruction[2] = {0, 0};
    state *state = &((*cpu).state);

    fetch(state, instruction);
    printf("Fetched: 0x%02X 0x%02X\n", instruction[0], instruction[1]);
    decode(instruction, &decoded_instruction);
    print_op(&decoded_instruction);
    execute(&decoded_instruction, state, cpu->peripherals);

    if (state->audio_timer > 0) 
    {
        cpu->peripherals->noise();
        state->audio_timer--;
    }
    // if (decoded_instruction.type == DISPLAY)
    print_screen(state->screen);
    for (int i = 0; i < REGISTER_COUNT; i++)
    {
        printf(" V%d: %x ", i, state->V[i]);
    }
    printf("I: %x PC: %x \n", state->I, state->PC);
}

void fetch(state *state, u_int8_t instruction[2])
{
    for (int i = 0; i < 2; i++)
        instruction[i] = state->memory[state->PC++];
}

void decode(u_int8_t instruction[2], op *decoded_op)
{
    u_int8_t op_type_major = (instruction[0] >> 4) & 0x0F;

    decoded_op->nnn = (((instruction[0]) & 0x0F) << 8) | instruction[1];
    decoded_op->nn = instruction[1];
    decoded_op->x = (instruction[0] & 0x0F);
    decoded_op->y = (instruction[1] & 0xF0) >> 4;
    decoded_op->n = instruction[1] & 0x0F;

    switch (op_type_major)
    {
    // Flow types
    case 0:
        switch (instruction[1] & 0xFF)
        {
        case 0xE0:
            decoded_op->type = CLEAR_DISPLAY;
            break;
        case 0xEE:
            decoded_op->type = RET;
        break;
        }

    case 1 ... 7:
    case 9 ... 0xD:
        decoded_op->type = op_type_lookup[op_type_major];
        break;

    case 8:
        decoded_op->type = bit_op_type_lookup[instruction[1] & 0xFF];
        break;
    
    case 0xE:
        switch (instruction[1] &0xFF) 
        {
        case 0x9E:
            decoded_op->type = SKIP_IF_KEY;
            break;
        case 0xA1:
            decoded_op->type = SKIP_IF_NKEY;
            break;
        }
    
    case 0xF:
        switch (instruction[1] & 0xFF)
        {
        case 0x07:
            decoded_op->type = GET_DELAY;
            break;
        case 0x0A:
            decoded_op->type = GET_KEY;
            break;
        case 0x15:
            decoded_op->type = SET_DELAY;
            break;
        case 0x18:
            decoded_op->type = SET_AUDIO;
            break;
        case 0x1E:
            decoded_op->type = ADVANCE_I;
            break;
        case 0x29:
            decoded_op->type = SET_I_HEX_SPRITE;
            break;
        case 0x33:
            decoded_op->type = BCD;
            break;
        case 0x55:
            decoded_op->type = REG_DUMP;
            break;
        case 0x65:
            decoded_op->type = REG_LOAD;
            break;
        }

    default:
        printf("Instruction with op_type: %i not yet implemented", op_type_major);
        break;
    }
}

void execute(op *decoded_op, state *state, peripherals *peripherals)
{
    u_int8_t *x = &(state->V[decoded_op->x]);
    u_int8_t *y = &(state->V[decoded_op->y]);

    switch (decoded_op->type)
    {
    case CLEAR_DISPLAY:
        memset(state->screen, 0, SCREEN_BYTES);
        break;
    case RET:
        state->PC = state->stack[state->SP--];
        break;
    case JUMP:
        state->PC = decoded_op->nnn;
        break;
    case CALL:
        state->stack[state->SP++] = state->PC;
        state->PC = decoded_op->nnn;
    case SET_REG:
        *x = decoded_op->nn;
        break;
    case ADD_REG:
        *x += decoded_op->nn;
        break;
    case SET_I_REG:
        state->I = decoded_op->nnn;
        break;
    case DISPLAY:
        display(state, decoded_op);
        break;
    case IF_EQ:
        state->PC += (*x == decoded_op->nn) ? 2 : 0;
        break;
    case IF_NEQ:
        state->PC += (*x != decoded_op->nn) ? 2 : 0;
        break;
    case IF_EQ_REG:
        state->PC += (*x == *y) ? 2 : 0;
        break;
    case SET_REG_BY_REG:
        *x = *y;
        break;
    case OR:
        *x |= *y;
        break;
    case AND:
        *x &= *y;
        break;
    case XOR:
        *x ^= *y;
        break;
    case ADD_BY_REG:
        // Carry
        state->V[0xF] = ((u_int8_t)(*x + *y) < *x || ((u_int8_t)(*x + *y)) < *y);
        *x += *y;
        break;
    case SUB:
        // Borrow
        state->V[0xF] = (*x > *y);
        *x -= *y;
        break;
    case SHIFT_RIGHT:
        state->V[0xF] = *x & 0x01;
        *x >>= 1;
        break;
    case SUBN:
        state->V[0xF] = (*y > *x);
        *x = *y - *x;
        break;
    case SHIFT_LEFT:
        state->V[0xF] = (*x & 0b10000000) >> 7;
        *x <<= 1;
        break;
    case SKIP_NEQ:
        state->PC += (*x != *y) ? 2 : 0;
        break;
    case BNNN:
        state->PC = decoded_op->nnn + state->V[0];
        break;
    case RANDOM:
        *x = peripherals->random() & decoded_op->nn;
        break;
    case SKIP_IF_KEY:
        state->PC += peripherals->is_key_pressed(*x) ? 2 : 0;
        break;
    case SKIP_IF_NKEY:
        state->PC += !peripherals->is_key_pressed(*x) ? 2 : 0;
        break;
    case GET_DELAY:
        *x = state->delay_timer;
        break;
    case GET_KEY:
        *x = peripherals->get_key_pressed();
        break;
    case SET_DELAY:
        state->delay_timer = *x;
        break;
    case SET_AUDIO:
        state->audio_timer = *x;
        break;
    case ADVANCE_I:
        state->I += *x;
        break;
    case SET_I_HEX_SPRITE:
        state->I = DIGIT_SPRITES_OFFSET + (*x * 5);
        break;
    case BCD:
        state->memory[state->I] = (*x / 100) % 10;    // 100's place
        state->memory[state->I + 1] = (*x / 10) % 10; // 10's place
        state->memory[state->I + 2] = *x % 10;        // 1's place
        break;
    case REG_DUMP:
        memcpy(&state->memory[state->I], state->V, REGISTER_COUNT);
        break;
    case REG_LOAD:
        memcpy(&state->V, &state->memory[state->I], REGISTER_COUNT);
        break;
    default:
        printf("Instruction with op_type %i not yet implemented", decoded_op->type);
    }
}

void display(state *state, op *decoded_op)
{
    u_int8_t *x = &(state->V[decoded_op->x]);
    u_int8_t *y = &(state->V[decoded_op->y]);

    // Used to determine if the operation resulted in ANY pixels were unset/toggled off
    int unsetPixel = 0;
    int start_idx = (((*y % SCREEN_H) * H_OFFSET) + *x);
    int end_idx = start_idx + (decoded_op->n * H_OFFSET);

    u_int8_t prev_screen_val;

    for (int i = start_idx, n = 0; i < end_idx; i += H_OFFSET, n++)
    {
        if (i >= SCREEN_BYTES)
            break;

        prev_screen_val = state->screen[i];
        state->screen[i] ^= state->memory[state->I + n];
        if (state->screen[i] < prev_screen_val)
            unsetPixel = 1;
    }
    printf("setPixel, %d", unsetPixel);
    state->V[0xF] = unsetPixel;
}

chip8 init(chip8_config *config)
{
    chip8 cpu;
    cpu.peripherals = config->peripherals;
    init_state(&cpu.state, config->memory, config->program);
    return cpu;
}

/**
 *  TODO: Actually implement some set up step
 */
void init_state(state *state, u_int8_t *memory, u_int8_t *program)
{
    // Point our memory to wherever has been allocated for us
    state->memory = memory;
    // Clear the contents of memory
    memset(state->memory, 0, RAM_SIZE);
    // Store sprite representation of hex digits to memory
    memcpy(&state->memory[DIGIT_SPRITES_OFFSET], digit_sprites_data, sizeof(digit_sprites_data));
    // Store the given program at the correct place in memory
    memcpy(&state->memory[PROGRAM_OFFSET], program, PROGRAM_SIZE);
    // Zero the stack
    memset(state->stack, 0, STACK_COUNT);

    // Init registers
    state->PC = PROGRAM_OFFSET;
    state->I = 0;
    state->SP = 0;
    memset(state->V, 0, REGISTER_COUNT);

    // Init peripherals
    state->audio_timer = 0;
    state->delay_timer = 0;
    memset(state->screen, 0, SCREEN_BYTES);
}

void print_screen(u_int8_t screen[SCREEN_BYTES])
{
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

void print_op(op *op)
{
    char op_type_str[16];
    switch (op->type)
    {

    case CLEAR_DISPLAY:
        strcpy(op_type_str, "CLEAR DISPLAY");
        break;

    case JUMP:
        strcpy(op_type_str, "JUMP");
        break;

    case SET_REG:
        strcpy(op_type_str, "SET REGISTER");
        break;

    case ADD_REG:
        strcpy(op_type_str, "ADD REGISTER");
        break;

    case SET_I_REG:
        strcpy(op_type_str, "SET I REGISTER");
        break;

    case DISPLAY:
        strcpy(op_type_str, "DISPLAY");
        break;

    case IF_EQ:
        strcpy(op_type_str, "IF EQUAL");
        break;

    case IF_NEQ:
        strcpy(op_type_str, "IF NOT EQUAL");
        break;

    case IF_EQ_REG:
        strcpy(op_type_str, "IF REG EQUAL");
        break;

    case SET_REG_BY_REG:
        strcpy(op_type_str, "SET X TO Y");
        break;

    case ADD_BY_REG:
        strcpy(op_type_str, "ADD X TO Y");
        break;

    case SUB:
        strcpy(op_type_str, "SUB Y FROM X");
        break;

    case SHIFT_RIGHT:
        strcpy(op_type_str, "SHIFT X RIGHT");
        break;

    case SUBN:
        strcpy(op_type_str, "SUB X FROM Y");
        break;

    case SHIFT_LEFT:
        strcpy(op_type_str, "SHIFT X LEFT");
        break;

    default:
        strcpy(op_type_str, "UNKNOWN");
        break;
    }

    printf("Command: %s NNN: %X, NN: %X X: %i Y: %i\n", op_type_str, op->nnn, op->nn, op->x, op->y);
}