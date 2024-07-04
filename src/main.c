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

#define UNSET_PIXEL 65 
#define SET_PIXEL 66 

// 4 KB
#define RAM_SIZE 4096

///
/// State
///

typedef struct state
{
    char memory[RAM_SIZE];
    char screen[SCREEN_BYTES];
    u_int16_t PC;
    u_int16_t I;
    char delay_timer;
    char audio_timer;
    char V[REGISTER_COUNT];
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
    DISPLAY
};

typedef struct op
{
    enum op_type type;
    char x;
    char y;
    u_int16_t nnn;
    char nn;
    char n;
} op;

void decode_nnn(char instruction[2], u_int16_t *nnn);
void decode_x(char instruction[2], char *x);
void decode_y(char instruction[2], char *y);

void init_state(state *state);

/**
 *  This function reads the next instruction from memory, based off of the given state's PC.
 *
 * @param state - The state of our emulator, which we'll use to read memory and PC
 * @param instruction - Ultimately the instruction we'll have read from memory
 */
void fetch(state *state, char instruction[2]);

/**
 * This function decodes the details of an instruction to extract its op code and operands.
 *
 * @param instruction - The two bytes that constitute the instruction being read (typically from PC)
 * @param decoded_op - A pointer to the well-defined format for an operation and its possible operands
 */
void decode(char instruction[2], op *decoded_op);

/**
 * Given a well formed decoded operation, this function will execute the operation against the
 * given state.
 *
 * @param decoded_op - The operation and its operands, to be executed
 * @param state - The state of our emulator to which the decoded_op will be run against
 */
void execute(op *decoded_op, state *state);


void print_screen(char screen[SCREEN_BYTES]);

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
    char instruction[2] = {0, 0};
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

void fetch(state *state, char instruction[2])
{
    for (int i = 0; i < 2; i++)
        instruction[i] = state->memory[state->PC++];
}

void decode(char instruction[2], op *decoded_op)
{
    char op_type_major = (instruction[0] >> 4) & 0x0F;

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
        }
        break;

    case 1:
        decoded_op->type = JUMP;
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
    char *x = &(state->V[decoded_op->x]);
    char *y = &(state->V[decoded_op->y]);

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
        memcpy(&state->screen, &state->memory[state->I], SCREEN_BYTES);
        int unsetPixel = 0;
        int start_idx = (( *y * H_OFFSET ) + *x);
        int end_idx = start_idx + (decoded_op->n * H_OFFSET);
        for (int i = start_idx; i <= end_idx; i += H_OFFSET) 
        {
            printf("i: %d, start: %d, end: %d\n", i, start_idx, end_idx);
            if (i >= SCREEN_BYTES) 
                break;

            state->screen[i] |= 0xFF; 
            if ( ((unsigned char) state->screen[i]) != 0xFF) 
                unsetPixel = 1;
        }
        state->V[0xF] = unsetPixel;
        break;
    default:
        printf("Instruction with op_type %i not yet implemented", decoded_op->type);
    }
}

void decode_nnn(char instruction[2], u_int16_t *nnn)
{
    // TODO: Change usage of chars to u_int8_t? This would save us the trouble of casting
    *nnn = ((((unsigned char) instruction[0]) & 0x0F) << 8) | (unsigned char) instruction[1];
}

void decode_x(char instruction[2], char *x)
{
    *x = (instruction[0] & 0x0F);
}

void decode_y(char instruction[2], char *y)
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
    // display (0, 1, 1)
    state->memory[12] = 0xD3;
    state->memory[13] = 0x41;
    // add reg (4, 1)
    state->memory[14] = 0x74;
    state->memory[15] = 0x01;
    // jump (12)
    state->memory[16] = 0x10;
    state->memory[17] = 0x0C;

    
    // for (int i = 0; i < SCREEN_BYTES; i++) 
        // state->memory[2000 + i] = 0xFF;

    state->PC = 0;
    for (int i = 0; i < REGISTER_COUNT; i++)
        state->V[i] = 0;
}

void print_screen(char screen[SCREEN_BYTES]) 
{
    char pixel_mask = 0b10000000;
    for (int i = 0; i < SCREEN_BYTES; i++)
    {
        if (i % H_OFFSET == 0)
            printf("\n");

        unsigned char pixels = (unsigned char) screen[i];
        printf("pixels: %x", pixels);
        for ( ; pixels > 0; pixels <<= 1) 
            // printf("Reached");
            printf("%c", ((pixels & pixel_mask) == 1) ? 'A': 'B');   
    
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

    default:
        strcpy(op_type_str, "UNKNOWN");
        break;
    }

    printf("Command: %s NNN: %X, NN: %X X: %i Y: %i\n", op_type_str, op->nnn, op->nn, op->x, op->y);
}