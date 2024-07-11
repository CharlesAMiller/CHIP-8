#include <stdint.h>

///
/// Constants
///

#define REGISTER_COUNT 16

#define SCREEN_W 64
#define SCREEN_H 32
#define SCREEN_BYTES ((SCREEN_W * SCREEN_H) / __CHAR_BIT__)
#define DIGIT_SPRITES_OFFSET 0

#define H_OFFSET (SCREEN_H / __CHAR_BIT__)

#define UNSET_PIXEL '-'
#define SET_PIXEL '#'

// 4 KB
#define RAM_SIZE 4096
#define PROGRAM_SIZE 0xDFF

#define PROGRAM_OFFSET 0x200

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
    IF_EQ_REG,
    SET_REG_BY_REG,
    OR,
    AND,
    XOR,
    ADD_BY_REG,
    SUB,
    SHIFT_RIGHT,
    SUBN,
    SHIFT_LEFT,
    SKIP_NEQ,
    BNNN,
    RANDOM,
    GET_DELAY,
    GET_KEY,
    SET_DELAY,
    SET_AUDIO,
    ADVANCE_I,
    SET_I_HEX_SPRITE,
    BCD,
    REG_DUMP,
    REG_LOAD,
};

extern enum op_type op_type_lookup[0xE];
extern enum op_type bit_op_type_lookup[9];
extern char *op_name_by_type[];
extern u_int8_t digit_sprites_data[0x50];

typedef struct op
{
    enum op_type type;
    u_int8_t x;
    u_int8_t y;
    u_int16_t nnn;
    u_int8_t nn;
    u_int8_t n;
} op;

///
/// State
///

typedef struct state
{
    u_int8_t *memory;
    u_int8_t screen[SCREEN_BYTES];
    u_int16_t PC;
    u_int16_t I;
    u_int8_t delay_timer;
    u_int8_t audio_timer;
    u_int8_t V[REGISTER_COUNT];
} state;

typedef struct peripherals
{
    // void (*display)(u_int8_t**);
    void (*noise)();
    u_int8_t (*random)();
    u_int8_t (*is_key_pressed)(u_int8_t);
    u_int8_t (*get_key_pressed)();
} peripherals;

typedef struct chip8
{
    state state;
    peripherals *peripherals;
} chip8;

typedef struct chip8_config 
{
    peripherals *peripherals;
    u_int8_t *memory;
    u_int8_t *program;
} chip8_config;

void init_state(state *state, u_int8_t *memory, u_int8_t *program);

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
void execute(op *decoded_op, state *state, peripherals *peripherals);

void run(chip8 *cpu);

chip8 init(chip8_config *config);

void print_screen(u_int8_t screen[SCREEN_BYTES]);

void display(state *state, op *decoded_op);

/**
 * A helper that pretty-prints the given operation
 *
 * @param op - The operation to be printed
 */
void print_op(op *op);
