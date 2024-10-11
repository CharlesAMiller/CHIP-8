/**
 * @file chip8.h
 * @brief The implementation of a [CHIP-8](https://en.wikipedia.org/wiki/CHIP-8) device
 *
 * This module broadly describes the implementation of the CHIP-8 device.

 * The module uses function pointers found in peripherals to keep the device logic more portable
 * and agnostic of particular medial libraries/dependencies.
 *
 * To use the module:
 * 1. Allocate memory suitable for the device
 * 2. Allocate and load the program/instructions to memory
 * 3. Wire your peripherals such that they align with the function pointer signatures described by the `peripherals` struct
 * 4. Call chip8_init with the aforementioned: memory, program memory, and peripherals and assign to a variable
 * 5. Iteratively call chip8_run, passing a reference to your previously assigned chip8 variable
 *
 * @see main.c for a well-defined example usage
 */
#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Check if SP is defined, and if so, undefine it
#ifdef SP
#undef SP
#endif

///
/// Constants
///

/**
 * @def REGISTER_COUNT
 * @brief The number of generic registers the device has
 */
#define REGISTER_COUNT 16

/**
 * @def STACK_COUNT
 * @brief The size of the device's call stack
 */
#define STACK_COUNT 16

/**
 * @def SCREEN_W
 * @brief The width of the device's screen
 */
#define SCREEN_W 64

/**
 * @def SCREEN_H
 * @brief The height of the device's screen
 */
#define SCREEN_H 32

/**
 * @def SCREEN_BYTES
 * @brief The number of bytes needed to represent the device's screen
 */
#define SCREEN_BYTES ((SCREEN_W * SCREEN_H) / __CHAR_BIT__)

/**
 * @def DIGIT_SPRITES_OFFSET
 * @brief The location in memory where the sprites of the hex digits should be stored
 */
#define DIGIT_SPRITES_OFFSET 0

/**
 * @def H_OFFSET
 * @brief How many bytes to a row of the screen.
 *   Multiplying by this offset effectively increments the y coordinate
 */
#define H_OFFSET (SCREEN_W / __CHAR_BIT__)

/**
 * @def RAM_SIZE
 * @brief The number of bytes representing the device's total memory
 *        4KB by default
 */
#define RAM_SIZE 4096

/**
 * @def PROGRAM_SIZE
 * @brief The number of bytes allocated for the program's memory
 */
#define PROGRAM_SIZE 0xDFF

/**
 * @def PROGRAM_OFFSET
 * @brief The location in the device's memory where the program should be stored
 */
#define PROGRAM_OFFSET 0x200

///
/// Instructions
///

/**
 * @enum op_type
 * @brief Enumeration of operation/instruction types supported by CHIP-8
 */
enum op_type
{
    /**
     * @brief Clears the screen buffer
     * peripheral: display
     */
    CLEAR_DISPLAY,
    /**
     * @brief Returns from the current function and sets PC to top of stack
     * modifies: SP, PC
     */
    RET,
    /**
     * @brief Jumps to a given instruction in memory
     * modifies: PC
     */
    JUMP,
    /**
     * @brief Jumps to a given instruction and adds current PC to top of stack
     * modifies: PC, SP
     */
    CALL,
    /**
     * @brief Sets register to a constant value
     * modifies: V[X]
     */
    SET_REG,
    /**
     * @brief Adds a constant value to given register
     * modifies: V[X]
     */
    ADD_REG,
    /**
     * @brief Sets the I register to a constant value
     * modifies: I
     */
    SET_I_REG,
    /**
     * @brief Compares the given register to a constant value, and skips following instruction if equal
     * modifies: PC
     */
    IF_EQ,
    /**
     * @brief Compares the given register to a constant value, and skips following instruction if not equal
     * modifies: PC
     */
    IF_NEQ,
    /**
     * @brief Compares the two given registers, and skips the following instruction if the two are equal
     * modifies: PC
     */
    IF_EQ_REG,
    /**
     * @brief Sets one given register to the value of another
     * modifies: V[X]
     */
    SET_REG_BY_REG,
    /**
     * @brief OR's (bitwise) the values of two given registers and assigns the result to the X register
     * modifies: V[X]
     */
    OR,
    /**
     * @brief AND's (bitwise) the values of two given registers and assigns the result to the X register
     * modifies: V[X]
     */
    AND,
    /**
     * @brief XOR the values of two given registers and assigns the result to the X register
     * modifies: V[X]
     */
    XOR,
    /**
     * @brief Adds the values of two given registers and assigns the result to the X register
     * V[0xF] is set to 1 if the operation resulted in an overflow/carry. 0 otherwise
     * modifies: V[X], V[0xF]
     */
    ADD_BY_REG,
    /**
     * @brief Subtracts the values of two given registers and assigns the result to the X register
     * V[0xF] is set to 1 if the operation did not result in an underflow/borrow. 0 otherwise
     * modifies: V[X], V[0xF]
     */
    SUB,
    /**
     * @brief The value of the given register is shifted right by one bit and reassigned to the register
     * V[0xF] is set to the value of the LSB, prior to the operation
     * modifies: V[X], V[0xF]
     */
    SHIFT_RIGHT,
    /**
     * @brief Subtracts the first given register from the second, and assigns the result to the first register
     * V[0xF] is set to 1 if the operation resulted in an underflow/borrow. 0 otherwise
     * modifies: V[X], V[0xF]
     */
    SUBN,
    /**
     * @brief The value of the given register is shifted left by one bit and reassigned to the register
     * V[0xF] is set to the value of the MSB, prior to the operation
     * modifies: V[X], V[0xF]
     */
    SHIFT_LEFT,
    /**
     * @brief Compares the value of the two given registers, and skips the next instruction if the two are unequal
     * modifies: PC
     */
    SKIP_NEQ,
    /**
     * @brief Sets the PC to V[0] plus a constant value
     * modifies: PC
     */
    BNNN,
    /**
     * @brief Generates a random number between 0-255 that is bitwise AND'd with a given constant and stores it to
     *  the given register
     * moidifies: V[X]
     * peripheral: random
     */
    RANDOM,
    /**
     * @brief Loads a given constant 'n' worth of bytes from memory at the I index,
     *  and draws the sprite to the V[X] and V[Y] position in the screen buffer
     * peripheral: display
     */
    DRAW_SPRITE,
    /**
     * @brief Skips the next instruction if the key stored in V[X] is being pressed
     * modifies: PC
     */
    SKIP_IF_KEY,
    /**
     * @brief Skips the next instruction if the key stored in V[X] is not being pressed
     */
    SKIP_IF_NKEY,
    /**
     * @brief Sets the given register V[X] to the current value of the delay_timer
     * modifies: V[X]
     */
    GET_DELAY,
    /**
     * @brief Performs a blocking operation that waits until a user presses a key on the keyboard,
     *  and stores the key to V[X]
     * modifies: V[X]
     */
    GET_KEY,
    /**
     * @brief Sets the delay_timer to the given register value V[X]
     * modifies: delay_timer
     */
    SET_DELAY,
    /**
     * @brief Sets the audio_timer to the given register value V[X]
     * modifies: audio_timer
     */
    SET_AUDIO,
    /**
     * @brief Adds the contents of the given register V[X] to the I register and stores the result to I
     * modifies: I
     */
    ADVANCE_I,
    /**
     * @brief Sets the I register to the location in memory that holds the ASCII hex digit
     *  stored in the given register V[X]
     * modifies: I
     */
    SET_I_HEX_SPRITE,
    /**
     * @brief Stores the binary coded decimal representation of the value in the given register V[X]
     *  to memory at the I register the hundreds digit of the value is stored at the first location,
     *  the tens at the second, and the ones at the third
     * modifies: MEMORY[I], MEMORY[I + 1], MEMORY[I + 2]
     */
    BCD,
    /**
     * @brief The contents of the registers are written to memory at the I register value.
     *  Only the X number of registers are written
     * modifies: MEMORY[I] ... MEMORY[X]
     */
    REG_DUMP,
    /**
     * @brief the contents of memory, starting at I, are read to the general registers.
     *  Only the X number of registers are read from memory
     * modifies: V[0] ... V[X]
     */
    REG_LOAD,
    /**
     * @brief Not an actual instruction. Just used as a placeholder.
     */
    NOOP,
};

/**
 * Lookup table that maps the first bit of an instruction to an instruction type.
 * This is not the case for all operations.
 */
extern enum op_type op_type_lookup[0xE];

/**
 * Lookup table that maps operations characterized by being a bitwise related operation
 * (has a first instruction bit of 8) to particular opeartions types
 */
extern enum op_type bit_op_type_lookup[0xF];

/**
 * A buffer that contains the ASCII representations of each hex digit
 * Each digit is 5 lines (and bytes) long
 */
extern uint8_t digit_sprites_data[0x50];

/**
 * @struct op
 * @brief A structure that contains the decoded information from an instruction
 *
 * The contents of the struct can be supplied to 'execute' to perform the operation
 */
typedef struct op
{
    /**
     * @brief The type of the operation.
     * @see op_type
     */
    enum op_type type;
    /**
     * @brief The X value of the instruction, typically refers to one of the general registers
     * e.g. 0x0X00;
     */
    uint8_t x;
    /**
     * @brief The Y value of the instruction, typically refers to one of the general registers
     * e.g. 0x00Y0;
     */
    uint8_t y;
    /**
     * @brief A 12 bit constant for the instruction, typically used for jump/set related instructions
     * e.g. 0x0NNN;
     */
    uint16_t nnn;
    /**
     * @brief An 8 bit constant for the instruction
     */
    uint8_t nn;
    /**
     * @brief A 4 bit constant, typically used for small offsets when X and Y are already defined
     */
    uint8_t n;
} op;

///
/// State
///

/**
 * @struct state
 * @brief The state of the CHIP-8 machine
 */
typedef struct state
{
    /**
     * @brief The pointer to the memory buffer. Should be set to the size specified by RAM_SIZE
     * @see RAM_SIZE
     */
    uint8_t *memory;
    /**
     * @brief A buffer for the contents of the machine's screen. Its size is determined by SCREEN_H x SCREEN_W
     */
    uint8_t screen[SCREEN_BYTES];
    /**
     * @brief The Program Counter register.
     * It points to the location in memory that holds the current instruction to be fetched
     */
    uint16_t PC;
    /**
     * @brief The Stack Pointer register. It points to an index in our stack.
     * It is incremented and decremented as RET and CALL operations are called
     */
    uint8_t SP;
    /**
     * @brief The Index register. It points to a location in memory, and is used for various operations
     */
    uint16_t I;
    /**
     * @brief The delay_timer. If positive, it is decremented by one each cycle of the cpu
     */
    uint8_t delay_timer;
    /**
     * @brief The audio_timer. If positive, it is decremented by one each cycle of the cpu.
     * Each cycle its value is positive, the audio peripheral is called to produce noise
     */
    uint8_t audio_timer;
    /**
     * @brief The Stack. It used to persist the PC as subroutines are branched to during CALL operations.
     * It is managed by the SP
     */
    uint16_t stack[STACK_COUNT];
    /**
     * @brief The General registers. There are 16 by default, and they are used to quickly store information
     * and perform as part of instructions
     */
    uint8_t V[REGISTER_COUNT];
} state;

/**
 * @struct peripherals
 * @brief Configurable function pointers that are provided as callbacks that may be used by the CHIP-8
 *  device during operation
 */
typedef struct peripherals
{
    /**
     * @brief The display peripheral that is called. It is provided the screen buffer as a parameter
     * It should draw the contents of the screen to some sort of display
     */
    void (*display)(uint8_t *);
    /**
     * @brief The audio/noiser peripheral that is periodically called
     * It should create some noise via some sounder
     */
    void (*noise)();
    /**
     * @brief The random number generator peripheral. It should produce a random byte of data.
     */
    uint8_t (*random)();
    /**
     * @brief The keyboard peripheral. It should determine if the key with the given code
     * was pressed. It should return a value of 1 if pressed, and 0 if it wasn't.
     */
    uint8_t (*is_key_pressed)(uint8_t);
    /**
     * @brief The keyboard peripheral. It should produce a blocking call that awaits the
     * press of a key. The keycode should be returned by this function.
     */
    uint8_t (*get_key_pressed)();
} peripherals;

/**
 * @struct chip8
 * @brief The data necessary to represent the CHIP-8 virtual device
 */
typedef struct chip8
{
    /**
     * @brief The state of the machine
     */
    state state;
    /**
     * @brief The given peripherals that may be accessed by the device
     */
    peripherals *peripherals;
} chip8;

/**
 * @struct chip8_config
 * @brief The configuration options for setting up a CHIP-8 device
 */
typedef struct chip8_config
{
    /**
     * @brief The peripherals that will be used by the CHIP-8 device
     */
    peripherals *peripherals;
    /**
     * @brief The pointer to the memory that has been allocated for the CHIP-8's
     */
    uint8_t *memory;
    /**
     * @brief The program instructions that have been read and will be loaded into the CHIP-8's memory
     */
    uint8_t *program;
} chip8_config;

/**
 * @brief initializes the given state with the provided memory and program/instructions
 *
 * It zeroes memory, registers, and sets up the memory appropriately. It maps the program
 * into the appropriate place in the device's memory
 */
void init_state(state *state, uint8_t *memory);

/**
 * @brief This function reads the next instruction from memory, based off of the given state's PC.
 *
 * @param state - The state of our emulator, which we'll use to read memory and PC
 * @param instruction - Ultimately the instruction we'll have read from memory
 */
void fetch(state *state, uint8_t instruction[2]);

/**
 * @brief This function decodes the details of an instruction to extract its op code and operands.
 *
 * @param instruction - The two bytes that constitute the instruction being read (typically from PC)
 * @param decoded_op - A pointer to the well-defined format for an operation and its possible operands
 */
void decode(uint8_t instruction[2], op *decoded_op);

/**
 * @brief Given a well formed decoded operation, this function will execute the operation against the
 * given state, and will call peripheral methods as necessary.
 *
 * @param decoded_op - The operation and its operands, to be executed
 * @param state - The state of our emulator to which the decoded_op will be run against
 * @param peripherals - The callbacks to peripheral functions (e.g. noiser)
 */
void execute(op *decoded_op, state *state, peripherals *peripherals);

/**
 * @brief Performs a single cycle/step against the given CHIP-8 instance
 *
 * 1. Fetch retrieves instruction and advances PC
 * 2. The instruction, and its operands, are decoded
 * 3. The decoded operation is executed against the state. Calling peripherals as necessary.
 *
 * @param cpu - The given CHIP-8 instance to run a cycle on
 */
void chip8_run(chip8 *cpu);

/**
 * @brief Initializes state for a well-defined CHIP-8 instance
 *
 * @param config - The configuration details necessary for initializing a virtual CHIP-8 device
 * @returns - A well formed CHIP-8 device
 */
chip8 chip8_init(chip8_config *config);

/**
 * @brief Loads a program into the CHIP-8's memory
 * 
 * @param cpu - The CHIP-8 instance to load the program into
 * @param program - The program to load into the CHIP-8's memory
 * @param program_size - The size of the program to load - should not exceed PROGRAM_SIZE
 */
void chip8_load_program(chip8 *cpu, uint8_t *program, size_t program_size);

/**
 * @brief Function called when decoding a DISPLAY operation
 * It sets the states screen buffer appropriately
 */
void chip8_display(state *state, op *decoded_op);

// If SP was previously defined, redefine it here
#ifdef SP_BACKUP
#define SP SP_BACKUP
#undef SP_BACKUP
#endif

#endif