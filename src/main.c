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
} op;

void decode_nnn(char instruction[2], u_int16_t *nnn);
void decode_x(char instruction[2], char *x);

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

    while (1)
    {
        fetch(&state, instruction);
        printf("Fetched: 0x%02X 0x%02X\n", instruction[0], instruction[1]);
        decode(instruction, &decoded_instruction);
        print_op(&decoded_instruction);
        execute(&decoded_instruction, &state);
        for (int i = 0; i < REGISTER_COUNT; i++)
        {
            printf(" V%d: %d ", i, state.V[i]);
        }
        printf("\n");
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
    char op_type = (instruction[0] >> 4) & 0x0F;
    switch (op_type)
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

    // Jump
    case 1:
        decoded_op->type = JUMP;
        decode_nnn(instruction, &decoded_op->nnn);
        break;

    // Set Register
    case 6:
        decoded_op->type = SET_REG;
        decode_x(instruction, &(decoded_op->x));
        decoded_op->nn = instruction[1];
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
        for (int i = 0; i < SCREEN_BYTES; i++)
            state->screen[i] = 0;
        break;
    case JUMP:
        state->PC = decoded_op->nnn;
        break;
    case SET_REG:
        state->V[decoded_op->x] = decoded_op->nn;
        break;
    default:
        printf("Instruction with op_type %i not yet implemented", decoded_op->type);
    }
}

void decode_nnn(char instruction[2], u_int16_t *nnn)
{
    *nnn = (((instruction[0] & 0x0F) << 4) << 6) | instruction[1];
}

void decode_x(char instruction[2], char *x)
{
    *x = (instruction[0] & 0x0F);
}

/**
 *  TODO: Actually implement some set up step
 */
void init_state(state *state)
{
    // Set up program memory with a dummy clear screen instruction that loops
    // clear screen
    state->memory[0] = 0x00;
    state->memory[1] = 0xE0;
    // set reg (1, 2)
    state->memory[2] = 0x61;
    state->memory[3] = 0x02;
    // jump (0)
    state->memory[4] = 0x10;
    state->memory[5] = 0x00;

    state->PC = 0;
    for (int i = 0; i < REGISTER_COUNT; i++)
        state->V[i] = 0;
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
    default:
        strcpy(op_type_str, "UNKNOWN");
        break;
    }

    printf("Command: %s NNN: %X, NN: %X X: %i Y: %i\n", op_type_str, op->nnn, op->nn, op->x, op->y);
}