/**
 *  This function reads the next instruction from memory, based off of the given state's PC.
 *
 * @param state - The state of our emulator, which we'll use to read memory and PC
 * @param instruction - Ultimately the instruction we'll have read from memory
 */
void fetch(state *state, char *instruction[2]);

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

