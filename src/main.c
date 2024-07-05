/** @file main.c
 * TODO:
 *  - Finish adding all op codes
 *  - Finish adding execution behavior
 *  - Separate into different files
 *  - Test that the code compiles to Arduino
 *  - Add functionality tests
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

///
/// Constants
///

#define REGISTER_COUNT 16

#define SCREEN_W 64
#define SCREEN_H 32
#define SCREEN_BYTES ((SCREEN_W * SCREEN_H) / __CHAR_BIT__)

#define H_OFFSET (SCREEN_H / __CHAR_BIT__)

#define UNSET_PIXEL '-'
#define SET_PIXEL '#' 

// 4 KB
#define RAM_SIZE 4096

///
/// State
///

typedef struct state
{
    u_int8_t memory[RAM_SIZE];
    u_int8_t screen[SCREEN_BYTES];
    u_int16_t PC;
    u_int16_t I;
    u_int8_t delay_timer;
    u_int8_t audio_timer;
    u_int8_t V[REGISTER_COUNT];
} state;

///
/// Instructions
///

// Instruction types
enum op_type
{
    CLEAR_DISPLAY,
    JUMP,
    SET_REG,
    ADD_REG,
    SET_I_REG,
    DISPLAY,
    IF_EQ,
    IF_NEQ,
};

typedef struct op
{
    enum op_type type;
    u_int8_t x;
    u_int8_t y;
    u_int16_t nnn;
    u_int8_t nn;
    u_int8_t n;
} op;

void decode_nnn(u_int8_t instruction[2], u_int16_t *nnn);
void decode_x(u_int8_t instruction[2], u_int8_t *x);
void decode_y(u_int8_t instruction[2], u_int8_t *y);

void init_state(state *state);

/**
 *  This function reads the next instruction from memory, based off of the given state's PC.
 *
 * @param state - The state of our emulator, which we'll use to read memory and PC
 * @param instruction - Ultimately the instruction we'll have read from memory
 */
void fetch(state *state, u_int8_t instruction[2]);

/**
 * This function decodes the details of an instruction to extract its op code and operands.
 *
 * @param instruction - The two bytes that constitute the instruction being read (typically from PC)
 * @param decoded_op - A pointer to the well-defined format for an operation and its possible operands
 */
void decode(u_int8_t instruction[2], op *decoded_op);

/**
 * Given a well formed decoded operation, this function will execute the operation against the
 * given state.
 *
 * @param decoded_op - The operation and its operands, to be executed
 * @param state - The state of our emulator to which the decoded_op will be run against
 */
void execute(op *decoded_op, state *state);

void display(state *state, op *decoded_op);

void print_screen(u_int8_t screen[SCREEN_BYTES]);

/**
 * A helper that pretty-prints the given operation
 *
 * @param op - The operation to be printed
 */
void print_op(op *op);

int main(int argc, char *argv[])
{
    // Setup
    state state;
    op decoded_instruction;
    u_int8_t instruction[2] = {0, 0};
    init_state(&state);

    while (1)
    {
        fetch(&state, instruction);
        printf("Fetched: 0x%02X 0x%02X\n", instruction[0], instruction[1]);
        decode(instruction, &decoded_instruction);
        print_op(&decoded_instruction);
        execute(&decoded_instruction, &state);
        if (decoded_instruction.type == DISPLAY)
            print_screen(state.screen);
        for (int i = 0; i < REGISTER_COUNT; i++)
        {
            printf(" V%d: %x ", i, state.V[i]);
        }
        printf("I: %x\n", state.I);
    }
    return 0;
}

void fetch(state *state, u_int8_t instruction[2])
{
    for (int i = 0; i < 2; i++)
        instruction[i] = state->memory[state->PC++];
}

void decode(u_int8_t instruction[2], op *decoded_op)
{
    u_int8_t op_type_major = (instruction[0] >> 4) & 0x0F;

    // TODO: Some simplification could be done here
    decode_nnn(instruction, &(decoded_op->nnn));
    decoded_op->nn = instruction[1];
    decode_x(instruction, &(decoded_op->x));
    decode_y(instruction, &(decoded_op->y));
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
        default: 
            printf("NOT YET IMPLEMENTED");
            break;
        }
        break;

    case 1:
        decoded_op->type = JUMP;
        break;

    case 3: 
        decoded_op->type = IF_EQ;
        break;

    case 4:
        decoded_op->type = IF_NEQ;
        break;

    case 6:
        decoded_op->type = SET_REG;
        break;

    case 7:
        decoded_op->type = ADD_REG;
        break;

    case 0xA:
        decoded_op->type = SET_I_REG;
        break;

    case 0xD:
        decoded_op->type = DISPLAY;
        break;

    default:
        printf("Instruction with op_type: %i not yet implemented", op_type_major);
        break;
    }
}

void execute(op *decoded_op, state *state)
{
    u_int8_t *x = &(state->V[decoded_op->x]);
    u_int8_t *y = &(state->V[decoded_op->y]);

    switch (decoded_op->type)
    {
    case CLEAR_DISPLAY:
        for (int i = 0; i < SCREEN_BYTES; i++)
            state->screen[i] = 0;
        break;
    case JUMP:
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
    case DISPLAY:
        display(state, decoded_op);
        break;
    case IF_EQ:
        state->PC += (*x == decoded_op->nn) ? 2 : 0;
        break;
    case IF_NEQ:
        state->PC += (*x != decoded_op->nn) ? 2 : 0;
        break;
    default:
        printf("Instruction with op_type %i not yet implemented", decoded_op->type);
    }
}

void display(state *state, op *decoded_op)
{
    u_int8_t *x = &(state->V[decoded_op->x]);
    u_int8_t *y = &(state->V[decoded_op->y]);

    memcpy(&state->screen, &state->memory[state->I], SCREEN_BYTES);
    // Used to determine if the operation resulted in ANY pixels were unset/toggled off
    int unsetPixel = 0;
    // int start_idx = (((*y % SCREEN_H) * H_OFFSET) + *x);
    int start_idx = 1;
    int end_idx = start_idx + (decoded_op->n * H_OFFSET);
    for (int i = start_idx; i < end_idx; i += H_OFFSET)
    {
        if (i >= SCREEN_BYTES)
            break;

        state->screen[i] ^= 0xFF;
        if (state->screen[i] != 0xFF)
            unsetPixel = 1;
    }
    printf("setPixel, %d", unsetPixel);
    state->V[0xF] = unsetPixel;
}

void decode_nnn(u_int8_t instruction[2], u_int16_t *nnn)
{
    // TODO: Change usage of chars to u_int8_t? This would save us the trouble of casting
    *nnn = (((instruction[0]) & 0x0F) << 8) | instruction[1];
}

void decode_x(u_int8_t instruction[2], u_int8_t *x)
{
    *x = (instruction[0] & 0x0F);
}

void decode_y(u_int8_t instruction[2], u_int8_t *y)
{
    *y = (instruction[1] & 0xF0) >> 4;
}

/**
 *  TODO: Actually implement some set up step
 */
void init_state(state *state)
{
    for (int i = 0; i < RAM_SIZE; i++)
        state->memory[i] = 0;

    // Set up program memory with a dummy program
    // clear screen
    state->memory[0] = 0x00;
    state->memory[1] = 0xE0;
    // set reg (1, 2)
    state->memory[2] = 0x61;
    state->memory[3] = 0x02;
    // add reg (1, 1)
    state->memory[4] = 0x71;
    state->memory[5] = 0x01;
    // set i reg (2000)
    state->memory[6] = 0xA7;
    state->memory[7] = 0xD0;
    // set reg (3, 32)
    state->memory[8] = 0x63;
    state->memory[9] = 0x20;
    // set reg (4, 0)
    state->memory[10] = 0x64;
    state->memory[11] = 0x00;
    // display (0, 1, 5)
    state->memory[12] = 0xD3;
    state->memory[13] = 0x45;
    // add reg (4, 1)
    state->memory[14] = 0x74;
    state->memory[15] = 0x01;
    // if eq (1, 3)
    state->memory[16] = 0x31;
    state->memory[17] = 0x03;
    // jump (14)
    state->memory[18] = 0x10;
    state->memory[19] = 0x0E;
    // jump (12)
    state->memory[20] = 0x10;
    state->memory[21] = 0x0C;

    // for (int i = 0; i < SCREEN_BYTES; i++)
    // state->memory[2000 + i] = 0xFF;

    state->PC = 0;
    for (int i = 0; i < REGISTER_COUNT; i++)
        state->V[i] = 0;
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
            printf("%c", ((pixels & pixel_mask) != 0) ? SET_PIXEL: UNSET_PIXEL);
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

    default:
        strcpy(op_type_str, "UNKNOWN");
        break;
    }

    printf("Command: %s NNN: %X, NN: %X X: %i Y: %i\n", op_type_str, op->nnn, op->nn, op->x, op->y);
}