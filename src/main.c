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

#define SCREEN_W 64
#define SCREEN_H 32
#define SCREEN_BYTES (SCREEN_W * SCREEN_H) / __CHAR_BIT__

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
    char V0, V1, V2, V3, V4, V5, V6, V7, V8, V9, VA, VB, VC, VD, VE, VF;
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
    char nnn[2];
} op;

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

    fetch(&state, instruction); 
    printf("Fetched: 0x%02X 0x%02X\n", instruction[0], instruction[1]);
    decode(instruction, &decoded_instruction);
    print_op(&decoded_instruction);
    execute(&decoded_instruction, &state);

    return 0;
}

void fetch(state *state, char instruction[2])
{
    for (int i = 0; i < 2; i++) {
        instruction[i] = state->memory[state->PC++];
    }
}

void decode(char instruction[2], op *decoded_op)
{
    char op_type = (instruction[0] & 0xF);
    switch (op_type)
    {
    // Flow types
    case 0:
        switch (instruction[1])
        {
        case 0xE0:
            decoded_op->type = CLEAR_DISPLAY;
            break;
        }
        break;

    default:
        printf("Instruction with op_type: %i not yet implemented", op_type);
        break;
    }
}

void execute(op *decoded_op, state *state)
{
    switch (decoded_op->type)
    {
    case CLEAR_DISPLAY:
        printf("Clearing display");
        for (int i = 0; i < SCREEN_BYTES; i++)
        {
            state->screen[i] = 0;
        }
    default:
        printf("Instruction with op_type %i not yet implemented", decoded_op->type);
    }
}

/**
 *  TODO: Actually implement some set up step 
 */
void init_state(state* state) {
    // Set up program memory with a dummy clear screen instruction
    state->memory[0] = 0x00;
    state->memory[1] = 0xE0;
    
    state->PC = 0;
    state->V0 = 0;
    // ...
}

void print_op(op *op)
{
    char op_type_str[16];
    switch (op->type)
    {

    case CLEAR_DISPLAY:
        strcpy(op_type_str, "CLEAR DISPLAY");
        break;

    default:
        strcpy(op_type_str, "UNKNOWN");
        break;
    }

    printf("Command: %s X: %i Y: %i\n", op_type_str, op->x, op->y);
}