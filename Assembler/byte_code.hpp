#ifndef TEA_BYTE_CODE_HEADER
#define TEA_BYTE_CODE_HEADER

#include <bits/stdc++.h>

using namespace std;

/**
 * @brief An enum containing all valid opcodes.
 */
enum Instruction : uint16_t {
	// Move instructions


	// Moves an 8-bit literal into a register.
	MOVE_8_INTO_REG,

	// Moves a 16-bit literal into a register.
	MOVE_16_INTO_REG,

	// Moves a 32-bit literal into a register.
	MOVE_32_INTO_REG,

	// Moves a 64-bit literal into a register.
	MOVE_64_INTO_REG,


	// Moves an 8-bit literal into a memory location.
	MOVE_8_INTO_MEM,

	// Moves a 16-bit literal into a memory location.
	MOVE_16_INTO_MEM,

	// Moves a 32-bit literal into a memory location.
	MOVE_32_INTO_MEM,

	// Moves a 64-bit literal into a memory location.
	MOVE_64_INTO_MEM,

	// Moves the contents of a 64-bit register into a 64-bit register.
	MOVE_REG_INTO_REG,


	// Moves the contents of an 8-bit register into a memory location.
	MOVE_REG_INTO_MEM_8,

	// Moves the contents of a 16-bit register into a memory location.
	MOVE_REG_INTO_MEM_16,

	// Moves the contents of a 32-bit register into a memory location.
	MOVE_REG_INTO_MEM_32,

	// Moves the contents of a 64-bit register into a memory location.
	MOVE_REG_INTO_MEM_64,


	// Moves 8 bits from a memory location into a register.
	MOVE_MEM_8_INTO_REG,

	// Moves 16 bits from a memory location into a register.
	MOVE_MEM_16_INTO_REG,

	// Moves 32 bits from a memory location into a register.
	MOVE_MEM_32_INTO_REG,

	// Moves 64 bits from a memory location into a register.
	MOVE_MEM_64_INTO_REG,


	// Moves 8 bits at a memory location specified by a register into a register.
	MOVE_REG_POINTER_8_INTO_REG,

	// Moves 16 bits at a memory location specified by a register into a register.
	MOVE_REG_POINTER_16_INTO_REG,

	// Moves 32 bits at a memory location specified by a register into a register.
	MOVE_REG_POINTER_32_INTO_REG,

	// Moves 64 bits at a memory location specified by a register into a register.
	MOVE_REG_POINTER_64_INTO_REG,


	// Moves 8 bits from register into a memory location specified by a register.
	MOVE_REG_INTO_REG_POINTER_8,

	// Moves 16 bits from register into a memory location specified by a register.
	MOVE_REG_INTO_REG_POINTER_16,

	// Moves 32 bits from register into a memory location specified by a register.
	MOVE_REG_INTO_REG_POINTER_32,

	// Moves 64 bits from register into a memory location specified by a register.
	MOVE_REG_INTO_REG_POINTER_64,


	// Moves 8 bits from an offset to the current stack frame into a register.
	MOVE_FRAME_OFFSET_8_INTO_REG,

	// Moves 16 bits from an offset to the current stack frame into a register.
	MOVE_FRAME_OFFSET_16_INTO_REG,

	// Moves 32 bits from an offset to the current stack frame into a register.
	MOVE_FRAME_OFFSET_32_INTO_REG,

	// Moves 64 bits from an offset to the current stack frame into a register.
	MOVE_FRAME_OFFSET_64_INTO_REG,


	// Moves 8 bits from a register into an offset to the current stack frame.
	MOVE_REG_INTO_FRAME_OFFSET_8,

	// Moves 16 bits from a register into an offset to the current stack frame.
	MOVE_REG_INTO_FRAME_OFFSET_16,

	// Moves 32 bits from a register into an offset to the current stack frame.
	MOVE_REG_INTO_FRAME_OFFSET_32,

	// Moves 64 bits from a register into an offset to the current stack frame.
	MOVE_REG_INTO_FRAME_OFFSET_64,


	// Moves 8 bits from an offset to the stack top into a register.
	MOVE_STACK_TOP_OFFSET_8_INTO_REG,

	// Moves 16 bits from an offset to the stack top into a register.
	MOVE_STACK_TOP_OFFSET_16_INTO_REG,

	// Moves 32 bits from an offset to the stack top into a register.
	MOVE_STACK_TOP_OFFSET_32_INTO_REG,

	// Moves 64 bits from an offset to the stack top into a register.
	MOVE_STACK_TOP_OFFSET_64_INTO_REG,


	// Moves 8 bits from a register into an offset to the stack top.
	MOVE_REG_INTO_STACK_TOP_OFFSET_8,

	// Moves 16 bits from a register into an offset to the stack top.
	MOVE_REG_INTO_STACK_TOP_OFFSET_16,

	// Moves 32 bits from a register into an offset to the stack top.
	MOVE_REG_INTO_STACK_TOP_OFFSET_32,

	// Moves 64 bits from a register into an offset to the stack top.
	MOVE_REG_INTO_STACK_TOP_OFFSET_64,


	// Moves the address to the top of the stack into a register.
	MOVE_STACK_TOP_ADDRESS_INTO_REG,


	// Mathematical operations


	// Adds an 8-bit literal to a register.
	ADD_8_INTO_REG,

	// Adds a 16-bit literal to a register.
	ADD_16_INTO_REG,

	// Adds a 32-bit literal to a register.
	ADD_32_INTO_REG,

	// Adds a 64-bit literal to a register.
	ADD_64_INTO_REG,

	// Adds the contents of a register to a register.
	ADD_REG_INTO_REG,


	// Subtracts an 8-bit literal from a register.
	SUBTRACT_8_FROM_REG,

	// Subtracts a 16-bit literal from a register.
	SUBTRACT_16_FROM_REG,

	// Subtracts a 32-bit literal from a register.
	SUBTRACT_32_FROM_REG,

	// Subtracts a 64-bit literal from a register.
	SUBTRACT_64_FROM_REG,

	// Subtracts the contents of a register from a register.
	SUBTRACT_REG_FROM_REG,


	// Multiplies an 8-bit literal with a register.
	MULTIPLY_8_INTO_REG,

	// Multiplies a 16-bit literal with a register.
	MULTIPLY_16_INTO_REG,

	// Multiplies a 32-bit literal with a register.
	MULTIPLY_32_INTO_REG,

	// Multiplies a 64-bit literal with a register.
	MULTIPLY_64_INTO_REG,

	// Multiplies the contents of a register with a register.
	MULTIPLY_REG_INTO_REG,


	// Divides a register by an 8-bit literal.
	DIVIDE_8_FROM_REG,

	// Divides a register by a 16-bit literal.
	DIVIDE_16_FROM_REG,

	// Divides a register by a 32-bit literal.
	DIVIDE_32_FROM_REG,

	// Divides a register by a 64-bit literal.
	DIVIDE_64_FROM_REG,

	// Divides a register by the contents of another register.
	DIVIDE_REG_FROM_REG,


	// Modulo a register by an 8-bit literal.
	TAKE_MODULO_8_OF_REG,

	// Modulo a register by a 16-bit literal.
	TAKE_MODULO_16_OF_REG,

	// Modulo a register by a 32-bit literal.
	TAKE_MODULO_32_OF_REG,

	// Modulo a register by a 64-bit literal.
	TAKE_MODULO_64_OF_REG,

	// Modulo a register by the contents of another register.
	TAKE_MODULO_REG_OF_REG,


	// ANDs a register with an 8-bit literal.
	AND_8_INTO_REG,

	// ANDs a register with a 16-bit literal.
	AND_16_INTO_REG,

	// ANDs a register with a 32-bit literal.
	AND_32_INTO_REG,

	// ANDs a register with a 64-bit literal.
	AND_64_INTO_REG,

	// ANDs a register with the contents of another register.
	AND_REG_INTO_REG,


	// ORs a register with an 8-bit literal.
	OR_8_INTO_REG,

	// ORs a register with a 16-bit literal.
	OR_16_INTO_REG,

	// ORs a register with a 32-bit literal.
	OR_32_INTO_REG,

	// ORs a register with a 64-bit literal.
	OR_64_INTO_REG,

	// ORs a register with the contents of another register.
	OR_REG_INTO_REG,


	// XORs a register with an 8-bit literal.
	XOR_8_INTO_REG,

	// XORs a register with a 16-bit literal.
	XOR_16_INTO_REG,

	// XORs a register with a 32-bit literal.
	XOR_32_INTO_REG,

	// XORs a register with a 64-bit literal.
	XOR_64_INTO_REG,

	// XORs a register with the contents of another register.
	XOR_REG_INTO_REG,


	// Shifts a register left by an 8-bit literal.
	LEFT_SHIFT_REG_BY_8,

	// Shifts a register left by the contents of another register.
	LEFT_SHIFT_REG_BY_REG,

	// Shifts a register right by a 8-bit literal.
	RIGHT_SHIFT_REG_BY_8,

	// Shifts a register right by the contents of another register.
	RIGHT_SHIFT_REG_BY_REG,


	// Increments a register by one.
	INCREMENT_REG,

	// Decrements a register by one.
	DECREMENT_REG,

	// Flips the bits in a register.
	NOT_REG,


	// Comparison and branching


	// Compares an 8-bit literal with a register.
	COMPARE_8_TO_REG,

	// Compares a 16-bit literal with a register.
	COMPARE_16_TO_REG,

	// Compares a 32-bit literal with a register.
	COMPARE_32_TO_REG,

	// Compares a 64-bit literal with a register.
	COMPARE_64_TO_REG,


	// Compares a register to an 8-bit literal.
	COMPARE_REG_TO_8,

	// Compares a register to a 16-bit literal.
	COMPARE_REG_TO_16,

	// Compares a register to a 32-bit literal.
	COMPARE_REG_TO_32,

	// Compares a register to a 64-bit literal.
	COMPARE_REG_TO_64,


	// Compares a register to a register.
	COMPARE_REG_TO_REG,


	// Sets a register to 1 if LHS > RHS.
	SET_REG_IF_GREATER,

	// Sets a register to 1 if LHS >= RHS.
	SET_REG_IF_GREATER_OR_EQUAL,

	// Sets a register to 1 if LHS < RHS.
	SET_REG_IF_LESS,

	// Sets a register to 1 if LHS <= RHS.
	SET_REG_IF_LESS_OR_EQUAL,

	// Sets a register to 1 if LHS == RHS.
	SET_REG_IF_EQUAL,

	// Sets a register to 1 if LHS != RHS.
	SET_REG_IF_NOT_EQUAL,


	// Jumps to a label.
	JUMP,

	// Jumps to a label if LHS > RHS.
	JUMP_IF_GREATER,

	// Jumps to a label if LHS >= RHS.
	JUMP_IF_GREATER_OR_EQUAL,

	// Jumps to a label if LHS < RHS.
	JUMP_IF_LESS,

	// Jumps to a label if LHS <= RHS.
	JUMP_IF_LESS_OR_EQUAL,

	// Jumps to a label if LHS == RHS.
	JUMP_IF_EQUAL,

	// Jumps to a label if LHS != RHS.
	JUMP_IF_NOT_EQUAL,


	// Pushing to and popping from stack


	// Pushes an 8-bit literal onto the stack.
	PUSH_8,

	// Pushes a 16-bit literal onto the stack.
	PUSH_16,

	// Pushes a 32-bit literal onto the stack.
	PUSH_32,

	// Pushes a 64-bit literal onto the stack.
	PUSH_64,


	// Pushes 8 bits of a register onto the stack.
	PUSH_REG_8,

	// Pushes 16 bits of a register onto the stack.
	PUSH_REG_16,

	// Pushes 32 bits of a register onto the stack.
	PUSH_REG_32,

	// Pushes 64 bits of a register onto the stack.
	PUSH_REG_64,


	// Pops an 8-bit literal from the stack into a register.
	POP_8_INTO_REG,

	// Pops a 16-bit literal from the stack into a register.
	POP_16_INTO_REG,

	// Pops a 32-bit literal from the stack into a register.
	POP_32_INTO_REG,

	// Pops a 64-bit literal from the stack into a register.
	POP_64_INTO_REG,


	// Functions


	// Calls a function.
	CALL,

	// Returns from a function.
	RETURN,


	// Allocate a number of bytes on the stack.
	ALLOCATE_STACK,

	// Deallocate a number of bytes on the stack.
	DEALLOCATE_STACK,


	// For debugging purposes


	// A comment that is ignored by the VM.
	COMMENT,

	// A label that is used for debugging.
	LABEL,


	// System calls


	// Prints a character to stdout.
	PRINT_CHAR,

	// Reads a character from stdin.
	GET_CHAR
};

/**
 * @brief Converts an instruction to a string.
 * @param instruction The instruction to convert.
 * @return A string representation of the instruction.
 */
const char *instruction_to_str(Instruction instruction)
{
	switch (instruction) {
		case MOVE_8_INTO_REG: return "MOVE_8_INTO_REG";
		case MOVE_16_INTO_REG: return "MOVE_16_INTO_REG";
		case MOVE_32_INTO_REG: return "MOVE_32_INTO_REG";
		case MOVE_64_INTO_REG: return "MOVE_64_INTO_REG";
		case MOVE_8_INTO_MEM: return "MOVE_8_INTO_MEM";
		case MOVE_16_INTO_MEM: return "MOVE_16_INTO_MEM";
		case MOVE_32_INTO_MEM: return "MOVE_32_INTO_MEM";
		case MOVE_64_INTO_MEM: return "MOVE_64_INTO_MEM";
		case MOVE_REG_INTO_REG: return "MOVE_REG_INTO_REG";
		case MOVE_REG_INTO_MEM_8: return "MOVE_REG_INTO_MEM_8";
		case MOVE_REG_INTO_MEM_16: return "MOVE_REG_INTO_MEM_16";
		case MOVE_REG_INTO_MEM_32: return "MOVE_REG_INTO_MEM_32";
		case MOVE_REG_INTO_MEM_64: return "MOVE_REG_INTO_MEM_64";
		case MOVE_MEM_8_INTO_REG: return "MOVE_MEM_8_INTO_REG";
		case MOVE_MEM_16_INTO_REG: return "MOVE_MEM_16_INTO_REG";
		case MOVE_MEM_32_INTO_REG: return "MOVE_MEM_32_INTO_REG";
		case MOVE_MEM_64_INTO_REG: return "MOVE_MEM_64_INTO_REG";
		case MOVE_REG_POINTER_8_INTO_REG: return "MOVE_REG_POINTER_8_INTO_REG";
		case MOVE_REG_POINTER_16_INTO_REG: return "MOVE_REG_POINTER_16_INTO_REG";
		case MOVE_REG_POINTER_32_INTO_REG: return "MOVE_REG_POINTER_32_INTO_REG";
		case MOVE_REG_POINTER_64_INTO_REG: return "MOVE_REG_POINTER_64_INTO_REG";
		case MOVE_REG_INTO_REG_POINTER_8: return "MOVE_REG_INTO_REG_POINTER_8";
		case MOVE_REG_INTO_REG_POINTER_16: return "MOVE_REG_INTO_REG_POINTER_16";
		case MOVE_REG_INTO_REG_POINTER_32: return "MOVE_REG_INTO_REG_POINTER_32";
		case MOVE_REG_INTO_REG_POINTER_64: return "MOVE_REG_INTO_REG_POINTER_64";
		case MOVE_FRAME_OFFSET_8_INTO_REG: return "MOVE_FRAME_OFFSET_8_INTO_REG";
		case MOVE_FRAME_OFFSET_16_INTO_REG: return "MOVE_FRAME_OFFSET_16_INTO_REG";
		case MOVE_FRAME_OFFSET_32_INTO_REG: return "MOVE_FRAME_OFFSET_32_INTO_REG";
		case MOVE_FRAME_OFFSET_64_INTO_REG: return "MOVE_FRAME_OFFSET_64_INTO_REG";
		case MOVE_REG_INTO_FRAME_OFFSET_8: return "MOVE_REG_INTO_FRAME_OFFSET_8";
		case MOVE_REG_INTO_FRAME_OFFSET_16: return "MOVE_REG_INTO_FRAME_OFFSET_16";
		case MOVE_REG_INTO_FRAME_OFFSET_32: return "MOVE_REG_INTO_FRAME_OFFSET_32";
		case MOVE_REG_INTO_FRAME_OFFSET_64: return "MOVE_REG_INTO_FRAME_OFFSET_64";
		case MOVE_STACK_TOP_OFFSET_8_INTO_REG: return "MOVE_STACK_TOP_OFFSET_8_INTO_REG";
		case MOVE_STACK_TOP_OFFSET_16_INTO_REG: return "MOVE_STACK_TOP_OFFSET_16_INTO_REG";
		case MOVE_STACK_TOP_OFFSET_32_INTO_REG: return "MOVE_STACK_TOP_OFFSET_32_INTO_REG";
		case MOVE_STACK_TOP_OFFSET_64_INTO_REG: return "MOVE_STACK_TOP_OFFSET_64_INTO_REG";
		case MOVE_REG_INTO_STACK_TOP_OFFSET_8: return "MOVE_REG_INTO_STACK_TOP_OFFSET_8";
		case MOVE_REG_INTO_STACK_TOP_OFFSET_16: return "MOVE_REG_INTO_STACK_TOP_OFFSET_16";
		case MOVE_REG_INTO_STACK_TOP_OFFSET_32: return "MOVE_REG_INTO_STACK_TOP_OFFSET_32";
		case MOVE_REG_INTO_STACK_TOP_OFFSET_64: return "MOVE_REG_INTO_STACK_TOP_OFFSET_64";
		case MOVE_STACK_TOP_ADDRESS_INTO_REG: return "MOVE_STACK_TOP_ADDRESS_INTO_REG";
		case ADD_8_INTO_REG: return "ADD_8_INTO_REG";
		case ADD_16_INTO_REG: return "ADD_16_INTO_REG";
		case ADD_32_INTO_REG: return "ADD_32_INTO_REG";
		case ADD_64_INTO_REG: return "ADD_64_INTO_REG";
		case ADD_REG_INTO_REG: return "ADD_REG_INTO_REG";
		case SUBTRACT_8_FROM_REG: return "SUBTRACT_8_FROM_REG";
		case SUBTRACT_16_FROM_REG: return "SUBTRACT_16_FROM_REG";
		case SUBTRACT_32_FROM_REG: return "SUBTRACT_32_FROM_REG";
		case SUBTRACT_64_FROM_REG: return "SUBTRACT_64_FROM_REG";
		case SUBTRACT_REG_FROM_REG: return "SUBTRACT_REG_FROM_REG";
		case MULTIPLY_8_INTO_REG: return "MULTIPLY_8_INTO_REG";
		case MULTIPLY_16_INTO_REG: return "MULTIPLY_16_INTO_REG";
		case MULTIPLY_32_INTO_REG: return "MULTIPLY_32_INTO_REG";
		case MULTIPLY_64_INTO_REG: return "MULTIPLY_64_INTO_REG";
		case MULTIPLY_REG_INTO_REG: return "MULTIPLY_REG_INTO_REG";
		case DIVIDE_8_FROM_REG: return "DIVIDE_8_FROM_REG";
		case DIVIDE_16_FROM_REG: return "DIVIDE_16_FROM_REG";
		case DIVIDE_32_FROM_REG: return "DIVIDE_32_FROM_REG";
		case DIVIDE_64_FROM_REG: return "DIVIDE_64_FROM_REG";
		case DIVIDE_REG_FROM_REG: return "DIVIDE_REG_FROM_REG";
		case TAKE_MODULO_8_OF_REG: return "TAKE_MODULO_8_OF_REG";
		case TAKE_MODULO_16_OF_REG: return "TAKE_MODULO_16_OF_REG";
		case TAKE_MODULO_32_OF_REG: return "TAKE_MODULO_32_OF_REG";
		case TAKE_MODULO_64_OF_REG: return "TAKE_MODULO_64_OF_REG";
		case TAKE_MODULO_REG_OF_REG: return "TAKE_MODULO_REG_OF_REG";
		case AND_8_INTO_REG: return "AND_8_INTO_REG";
		case AND_16_INTO_REG: return "AND_16_INTO_REG";
		case AND_32_INTO_REG: return "AND_32_INTO_REG";
		case AND_64_INTO_REG: return "AND_64_INTO_REG";
		case AND_REG_INTO_REG: return "AND_REG_INTO_REG";
		case OR_8_INTO_REG: return "OR_8_INTO_REG";
		case OR_16_INTO_REG: return "OR_16_INTO_REG";
		case OR_32_INTO_REG: return "OR_32_INTO_REG";
		case OR_64_INTO_REG: return "OR_64_INTO_REG";
		case OR_REG_INTO_REG: return "OR_REG_INTO_REG";
		case XOR_8_INTO_REG: return "XOR_8_INTO_REG";
		case XOR_16_INTO_REG: return "XOR_16_INTO_REG";
		case XOR_32_INTO_REG: return "XOR_32_INTO_REG";
		case XOR_64_INTO_REG: return "XOR_64_INTO_REG";
		case XOR_REG_INTO_REG: return "XOR_REG_INTO_REG";
		case LEFT_SHIFT_REG_BY_8: return "LEFT_SHIFT_REG_BY_8";
		case LEFT_SHIFT_REG_BY_REG: return "LEFT_SHIFT_REG_BY_REG";
		case RIGHT_SHIFT_REG_BY_8: return "RIGHT_SHIFT_REG_BY_8";
		case RIGHT_SHIFT_REG_BY_REG: return "RIGHT_SHIFT_REG_BY_REG";
		case INCREMENT_REG: return "INCREMENT_REG";
		case DECREMENT_REG: return "DECREMENT_REG";
		case NOT_REG: return "NOT_REG";
		case COMPARE_8_TO_REG: return "COMPARE_8_TO_REG";
		case COMPARE_16_TO_REG: return "COMPARE_16_TO_REG";
		case COMPARE_32_TO_REG: return "COMPARE_32_TO_REG";
		case COMPARE_64_TO_REG: return "COMPARE_64_TO_REG";
		case COMPARE_REG_TO_8: return "COMPARE_REG_TO_8";
		case COMPARE_REG_TO_16: return "COMPARE_REG_TO_16";
		case COMPARE_REG_TO_32: return "COMPARE_REG_TO_32";
		case COMPARE_REG_TO_64: return "COMPARE_REG_TO_64";
		case COMPARE_REG_TO_REG: return "COMPARE_REG_TO_REG";
		case SET_REG_IF_GREATER: return "SET_REG_IF_GREATER";
		case SET_REG_IF_GREATER_OR_EQUAL: return "SET_REG_IF_GREATER_OR_EQUAL";
		case SET_REG_IF_LESS: return "SET_REG_IF_LESS";
		case SET_REG_IF_LESS_OR_EQUAL: return "SET_REG_IF_LESS_OR_EQUAL";
		case SET_REG_IF_EQUAL: return "SET_REG_IF_EQUAL";
		case SET_REG_IF_NOT_EQUAL: return "SET_REG_IF_NOT_EQUAL";
		case JUMP: return "JUMP";
		case JUMP_IF_GREATER: return "JUMP_IF_GREATER";
		case JUMP_IF_GREATER_OR_EQUAL: return "JUMP_IF_GREATER_OR_EQUAL";
		case JUMP_IF_LESS: return "JUMP_IF_LESS";
		case JUMP_IF_LESS_OR_EQUAL: return "JUMP_IF_LESS_OR_EQUAL";
		case JUMP_IF_EQUAL: return "JUMP_IF_EQUAL";
		case JUMP_IF_NOT_EQUAL: return "JUMP_IF_NOT_EQUAL";
		case PUSH_8: return "PUSH_8";
		case PUSH_16: return "PUSH_16";
		case PUSH_32: return "PUSH_32";
		case PUSH_64: return "PUSH_64";
		case PUSH_REG_8: return "PUSH_REG_8";
		case PUSH_REG_16: return "PUSH_REG_16";
		case PUSH_REG_32: return "PUSH_REG_32";
		case PUSH_REG_64: return "PUSH_REG_64";
		case POP_8_INTO_REG: return "POP_8_INTO_REG";
		case POP_16_INTO_REG: return "POP_16_INTO_REG";
		case POP_32_INTO_REG: return "POP_32_INTO_REG";
		case POP_64_INTO_REG: return "POP_64_INTO_REG";
		case CALL: return "CALL";
		case RETURN: return "RETURN";
		case ALLOCATE_STACK: return "ALLOCATE_STACK";
		case DEALLOCATE_STACK: return "DEALLOCATE_STACK";
		case COMMENT: return "COMMENT";
		case LABEL: return "LABEL";
		case PRINT_CHAR: return "PRINT_CHAR";
		case GET_CHAR: return "GET_CHAR";
		default: return "UNDEFINED";
	}
}

/**
 * @brief An enum of all valid argument types.
 */
enum ArgumentType : uint8_t {
	REG, ABS_ADDR, REL_ADDR,
	LIT_8, LIT_16, LIT_32, LIT_64,
	NULL_TERMINATED_STRING
};

/**
 * @brief Returns the allowed argument types for a given instruction.
 * @param instruction The instruction to get the argument types for.
 */
vector<ArgumentType> instruction_arg_types(Instruction instruction)
{
	switch (instruction) {
		case MOVE_8_INTO_REG: return { LIT_8, REG };
		case MOVE_16_INTO_REG: return { LIT_16, REG };
		case MOVE_32_INTO_REG: return { LIT_32, REG };
		case MOVE_64_INTO_REG: return { LIT_64, REG };
		case MOVE_8_INTO_MEM: return { LIT_8, ABS_ADDR };
		case MOVE_16_INTO_MEM: return { LIT_16, ABS_ADDR };
		case MOVE_32_INTO_MEM: return { LIT_32, ABS_ADDR };
		case MOVE_64_INTO_MEM: return { LIT_64, ABS_ADDR };
		case MOVE_REG_INTO_REG: return { REG, REG };
		case MOVE_REG_INTO_MEM_8: return { REG, ABS_ADDR };
		case MOVE_REG_INTO_MEM_16: return { REG, ABS_ADDR };
		case MOVE_REG_INTO_MEM_32: return { REG, ABS_ADDR };
		case MOVE_REG_INTO_MEM_64: return { REG, ABS_ADDR };
		case MOVE_MEM_8_INTO_REG: return { ABS_ADDR, REG };
		case MOVE_MEM_16_INTO_REG: return { ABS_ADDR, REG };
		case MOVE_MEM_32_INTO_REG: return { ABS_ADDR, REG };
		case MOVE_MEM_64_INTO_REG: return { ABS_ADDR, REG };
		case MOVE_REG_POINTER_8_INTO_REG: return { REG, REG };
		case MOVE_REG_POINTER_16_INTO_REG: return { REG, REG };
		case MOVE_REG_POINTER_32_INTO_REG: return { REG, REG };
		case MOVE_REG_POINTER_64_INTO_REG: return { REG, REG };
		case MOVE_REG_INTO_REG_POINTER_8: return { REG, REG };
		case MOVE_REG_INTO_REG_POINTER_16: return { REG, REG };
		case MOVE_REG_INTO_REG_POINTER_32: return { REG, REG };
		case MOVE_REG_INTO_REG_POINTER_64: return { REG, REG };
		case MOVE_FRAME_OFFSET_8_INTO_REG: return { LIT_64, REG };
		case MOVE_FRAME_OFFSET_16_INTO_REG: return { LIT_64, REG };
		case MOVE_FRAME_OFFSET_32_INTO_REG: return { LIT_64, REG };
		case MOVE_FRAME_OFFSET_64_INTO_REG: return { LIT_64, REG };
		case MOVE_REG_INTO_FRAME_OFFSET_8: return { REG, LIT_64 };
		case MOVE_REG_INTO_FRAME_OFFSET_16: return { REG, LIT_64 };
		case MOVE_REG_INTO_FRAME_OFFSET_32: return { REG, LIT_64 };
		case MOVE_REG_INTO_FRAME_OFFSET_64: return { REG, LIT_64 };
		case MOVE_STACK_TOP_OFFSET_8_INTO_REG: return { LIT_64, REG };
		case MOVE_STACK_TOP_OFFSET_16_INTO_REG: return { LIT_64, REG };
		case MOVE_STACK_TOP_OFFSET_32_INTO_REG: return { LIT_64, REG };
		case MOVE_STACK_TOP_OFFSET_64_INTO_REG: return { LIT_64, REG };
		case MOVE_REG_INTO_STACK_TOP_OFFSET_8: return { REG, LIT_64 };
		case MOVE_REG_INTO_STACK_TOP_OFFSET_16: return { REG, LIT_64 };
		case MOVE_REG_INTO_STACK_TOP_OFFSET_32: return { REG, LIT_64 };
		case MOVE_REG_INTO_STACK_TOP_OFFSET_64: return { REG, LIT_64 };
		case MOVE_STACK_TOP_ADDRESS_INTO_REG: return { REG };
		case ADD_8_INTO_REG: return { LIT_8, REG };
		case ADD_16_INTO_REG: return { LIT_16, REG };
		case ADD_32_INTO_REG: return { LIT_32, REG };
		case ADD_64_INTO_REG: return { LIT_64, REG };
		case ADD_REG_INTO_REG: return { REG, REG };
		case SUBTRACT_8_FROM_REG: return { LIT_8, REG };
		case SUBTRACT_16_FROM_REG: return { LIT_16, REG };
		case SUBTRACT_32_FROM_REG: return { LIT_32, REG };
		case SUBTRACT_64_FROM_REG: return { LIT_64, REG };
		case SUBTRACT_REG_FROM_REG: return { REG, REG };
		case MULTIPLY_8_INTO_REG: return { LIT_8, REG };
		case MULTIPLY_16_INTO_REG: return { LIT_16, REG };
		case MULTIPLY_32_INTO_REG: return { LIT_32, REG };
		case MULTIPLY_64_INTO_REG: return { LIT_64, REG };
		case MULTIPLY_REG_INTO_REG: return { REG, REG  };
		case DIVIDE_8_FROM_REG: return { LIT_8, REG };
		case DIVIDE_16_FROM_REG: return { LIT_16, REG };
		case DIVIDE_32_FROM_REG: return { LIT_32, REG };
		case DIVIDE_64_FROM_REG: return { LIT_64, REG };
		case DIVIDE_REG_FROM_REG: return { REG, REG  };
		case TAKE_MODULO_8_OF_REG: return { LIT_8, REG };
		case TAKE_MODULO_16_OF_REG: return { LIT_16, REG };
		case TAKE_MODULO_32_OF_REG: return { LIT_32, REG };
		case TAKE_MODULO_64_OF_REG: return { LIT_64, REG };
		case TAKE_MODULO_REG_OF_REG: return { REG, REG  };
		case AND_8_INTO_REG: return { LIT_8, REG };
		case AND_16_INTO_REG: return { LIT_16, REG };
		case AND_32_INTO_REG: return { LIT_32, REG };
		case AND_64_INTO_REG: return { LIT_64, REG };
		case AND_REG_INTO_REG: return { REG, REG  };
		case OR_8_INTO_REG: return { LIT_8, REG };
		case OR_16_INTO_REG: return { LIT_16, REG };
		case OR_32_INTO_REG: return { LIT_32, REG };
		case OR_64_INTO_REG: return { LIT_64, REG };
		case OR_REG_INTO_REG: return { REG, REG  };
		case XOR_8_INTO_REG: return { LIT_8, REG };
		case XOR_16_INTO_REG: return { LIT_16, REG };
		case XOR_32_INTO_REG: return { LIT_32, REG };
		case XOR_64_INTO_REG: return { LIT_64, REG };
		case XOR_REG_INTO_REG: return { REG, REG  };
		case LEFT_SHIFT_REG_BY_8: return { REG, LIT_8 };
		case LEFT_SHIFT_REG_BY_REG: return { REG, REG };
		case RIGHT_SHIFT_REG_BY_8: return { REG, LIT_8 };
		case RIGHT_SHIFT_REG_BY_REG: return { REG, REG };
		case INCREMENT_REG: return { REG };
		case DECREMENT_REG: return { REG };
		case NOT_REG: return { REG };
		case COMPARE_8_TO_REG: return { LIT_8, REG };
		case COMPARE_16_TO_REG: return { LIT_16, REG };
		case COMPARE_32_TO_REG: return { LIT_32, REG };
		case COMPARE_64_TO_REG: return { LIT_64, REG };
		case COMPARE_REG_TO_8: return { REG, LIT_8 };
		case COMPARE_REG_TO_16: return { REG, LIT_16 };
		case COMPARE_REG_TO_32: return { REG, LIT_32 };
		case COMPARE_REG_TO_64: return { REG, LIT_64 };
		case COMPARE_REG_TO_REG: return { REG, REG };
		case SET_REG_IF_GREATER: return { REG };
		case SET_REG_IF_GREATER_OR_EQUAL: return { REG };
		case SET_REG_IF_LESS: return { REG };
		case SET_REG_IF_LESS_OR_EQUAL: return { REG };
		case SET_REG_IF_EQUAL: return { REG };
		case SET_REG_IF_NOT_EQUAL: return { REG };
		case JUMP: return { REL_ADDR };
		case JUMP_IF_GREATER: return { REL_ADDR };
		case JUMP_IF_GREATER_OR_EQUAL: return { REL_ADDR };
		case JUMP_IF_LESS: return { REL_ADDR };
		case JUMP_IF_LESS_OR_EQUAL: return { REL_ADDR };
		case JUMP_IF_EQUAL: return { REL_ADDR };
		case JUMP_IF_NOT_EQUAL: return { REL_ADDR };
		case PUSH_8: return { LIT_8 };
		case PUSH_16: return { LIT_16 };
		case PUSH_32: return { LIT_32 };
		case PUSH_64: return { LIT_64 };
		case PUSH_REG_8: return { REG };
		case PUSH_REG_16: return { REG };
		case PUSH_REG_32: return { REG };
		case PUSH_REG_64: return { REG };
		case POP_8_INTO_REG: return { REG };
		case POP_16_INTO_REG: return { REG };
		case POP_32_INTO_REG: return { REG };
		case POP_64_INTO_REG: return { REG };
		case CALL: return { REL_ADDR };
		case RETURN: return {};
		case ALLOCATE_STACK: return { LIT_64 };
		case DEALLOCATE_STACK: return { LIT_64 };
		case COMMENT: return { NULL_TERMINATED_STRING };
		case LABEL: return { NULL_TERMINATED_STRING };
		case PRINT_CHAR: return { REG };
		case GET_CHAR: return { ABS_ADDR };
		default: return {};
	}
}

#endif