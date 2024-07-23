/**
 * @file chip8.c
 * @brief This module implements the logic for the CHIP-8 device
 */
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
    [0xD] = DRAW_SPRITE};

enum op_type bit_op_type_lookup[0xF] = {
    [0] = SET_REG_BY_REG,
    [1] = OR,
    [2] = AND,
    [3] = XOR,
    [4] = ADD_BY_REG,
    [5] = SUB,
    [6] = SHIFT_RIGHT,
    [7] = SUBN,
    [0xE] = SHIFT_LEFT};

uint8_t digit_sprites_data[0x50] = {
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

void chip8_run(chip8 *cpu)
{
    static op decoded_instruction;
    static uint8_t instruction[2] = {0, 0};
    state *state = &((*cpu).state);

    fetch(state, instruction);
    decode(instruction, &decoded_instruction);
    execute(&decoded_instruction, state, cpu->peripherals);

    if (state->audio_timer > 0)
    {
        cpu->peripherals->noise();
        state->audio_timer--;
    }

    if (state->delay_timer > 0)
        state->delay_timer--;
}

void fetch(state *state, uint8_t instruction[2])
{
    for (int i = 0; i < 2; i++)
        instruction[i] = state->memory[state->PC++];
}

void decode(uint8_t instruction[2], op *decoded_op)
{
    uint8_t op_type_major = (instruction[0] >> 4) & 0x0F;               // 0xN000;
    decoded_op->nnn = (((instruction[0]) & 0x0F) << 8) | instruction[1]; // 0x0NNN;
    decoded_op->nn = instruction[1];                                     // 0x00NN;
    decoded_op->x = (instruction[0] & 0x0F);                             // 0x0X00;
    decoded_op->y = (instruction[1] & 0xF0) >> 4;                        // 0x00Y0;
    decoded_op->n = instruction[1] & 0x0F;                               // 0x000N;

    switch (op_type_major)
    {
    case 0:
        switch (decoded_op->nn)
        {
        case 0xE0:
            decoded_op->type = CLEAR_DISPLAY;
            break;
        case 0xEE:
            decoded_op->type = RET;
            break;
        }
        break;

    case 1 ... 7:
    case 9 ... 0xD:
        decoded_op->type = op_type_lookup[op_type_major];
        break;

    case 8:
        decoded_op->type = bit_op_type_lookup[decoded_op->n];
        break;

    case 0xE:
        switch (decoded_op->nn)
        {
        case 0x9E:
            decoded_op->type = SKIP_IF_KEY;
            break;
        case 0xA1:
            decoded_op->type = SKIP_IF_NKEY;
            break;
        }
        break;

    case 0xF:
        switch (decoded_op->nn)
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
        break;

    default:
        // The operation is not implemented or is invalid
        break;
    }
}

void execute(op *decoded_op, state *state, peripherals *peripherals)
{
    uint8_t *x = &(state->V[decoded_op->x]);
    uint8_t *y = &(state->V[decoded_op->y]);

    uint8_t temp; 

    switch (decoded_op->type)
    {
    case CLEAR_DISPLAY:
        memset(state->screen, 0, SCREEN_BYTES);
        peripherals->display(state->screen);
        break;
    case RET:
        state->SP -= (state->SP > 0) ? 1 : 0; // Min SP is 0
        state->PC = state->stack[state->SP];
        break;
    case JUMP:
        state->PC = decoded_op->nnn;
        break;
    case CALL:
        state->stack[state->SP] = state->PC;
        state->SP += (state->SP < 15) ? 1 : 0; // Max SP is 15
        state->PC = decoded_op->nnn;
        break;
    case SET_REG:
        *x = decoded_op->nn;
        break;
    case ADD_REG:
        *x += decoded_op->nn;
        break;
    case SET_I_REG:
        state->I = decoded_op->nnn;
        break;
    case DRAW_SPRITE:
        display(state, decoded_op);
        peripherals->display(state->screen);
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
        temp = ((uint8_t)(*x + *y) < *x || ((uint8_t)(*x + *y)) < *y); // Carry
        *x += *y;
        state->V[0xF] = temp;
        break;
    case SUB:
        temp = (*x > *y); // NOT borrow
        *x -= *y;
        state->V[0xF] = temp;
        break;
    case SHIFT_RIGHT:
        temp = *x & 0x01; // LSB was set
        *x >>= 1;
        state->V[0xF] = temp;
        break;
    case SUBN:
        temp = (*y > *x);
        *x = *y - *x;
        state->V[0xF] = temp;
        break;
    case SHIFT_LEFT:
        temp = (*x & 0b10000000) >> 7; // MSB was set
        *x <<= 1;
        state->V[0xF] = temp;
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
        state->PC += (peripherals->is_key_pressed(*x) == 1) ? 2 : 0;
        break;
    case SKIP_IF_NKEY:
        state->PC += (peripherals->is_key_pressed(*x) == 0) ? 2 : 0;
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
        memcpy(&state->memory[state->I], state->V, decoded_op->x + 1);
        break;
    case REG_LOAD:
        memcpy(&state->V, &state->memory[state->I], decoded_op->x + 1);
        break;
    default:
        // The instruction is either not yet implemented or it is invalid
        break;
    }
}

void display(state *state, op *decoded_op)
{
    // Because the screen with is 64, there are 8 bytes to each row,
    // this tells us which byte our given x pixel is in
    uint8_t x = ((state->V[decoded_op->x]) % SCREEN_W) / 8;
    // This tells us how many bits into our byte we should start drawing the sprite
    uint8_t x_off = ((state->V[decoded_op->x]) % SCREEN_W) % 8;
    uint8_t y = (state->V[decoded_op->y]) % SCREEN_H;

    // Used to determine if the operation resulted in ANY pixels were unset/toggled off
    int unset_pixel = 0;
    int start_idx = ((y * H_OFFSET) + x);
    int end_idx = start_idx + (decoded_op->n * H_OFFSET);

    uint8_t screen_buffer;

    for (int i = start_idx, n = 0; i < end_idx; i += H_OFFSET, n++)
    {
        if (i >= SCREEN_BYTES)
            break;

        screen_buffer = state->screen[i] ^ (state->memory[state->I + n] >> x_off);
        for (int j = 8; j > 0; j--)
            if ( (state->screen[i] & (1 << j)) != 0 && (screen_buffer & (1 << j)) == 0)
                unset_pixel = 1;

        state->screen[i] = screen_buffer;

        // Depending on the x coord, the sprite can span two bytes.
        // So if we just drew 7 pixels to the previous byte, we'll draw the remaining 1 bit
        // to the screen's following byte.
        if (x_off > 0 && (i + 1) % H_OFFSET != 0)
        {
            screen_buffer = state->screen[i + 1] ^ (state->memory[state->I + n] << (8 - x_off));
            for (int j = 8; j > 0; j--)
                if ( (state->screen[i + 1] & (1 << j)) != 0 && (screen_buffer & (1 << j)) == 0)
                    unset_pixel = 1;

            state->screen[i + 1] = screen_buffer;
        }
    }
    state->V[0xF] = unset_pixel;
}

chip8 chip8_init(chip8_config *config)
{
    chip8 cpu;
    cpu.peripherals = config->peripherals;
    init_state(&cpu.state, config->memory, config->program);
    return cpu;
} 

void init_state(state *state, uint8_t *memory, uint8_t *program)
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