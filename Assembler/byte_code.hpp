#ifndef TEA_BYTE_CODE_HEADER
#define TEA_BYTE_CODE_HEADER

#include <bits/stdc++.h>

using namespace std;

enum Instruction : uint16_t {
	// Move instructions


	MOVE_8_INTO_REG,
	MOVE_16_INTO_REG,
	MOVE_32_INTO_REG,
	MOVE_64_INTO_REG,

	MOVE_8_INTO_MEM,
	MOVE_16_INTO_MEM,
	MOVE_32_INTO_MEM,
	MOVE_64_INTO_MEM,

	MOVE_REG_INTO_REG,

	MOVE_REG_INTO_MEM_8,
	MOVE_REG_INTO_MEM_16,
	MOVE_REG_INTO_MEM_32,
	MOVE_REG_INTO_MEM_64,

	MOVE_MEM_8_INTO_REG,
	MOVE_MEM_16_INTO_REG,
	MOVE_MEM_32_INTO_REG,
	MOVE_MEM_64_INTO_REG,

	MOVE_REG_POINTER_8_INTO_REG,
	MOVE_REG_POINTER_16_INTO_REG,
	MOVE_REG_POINTER_32_INTO_REG,
	MOVE_REG_POINTER_64_INTO_REG,

	MOVE_REG_INTO_REG_POINTER_8,
	MOVE_REG_INTO_REG_POINTER_16,
	MOVE_REG_INTO_REG_POINTER_32,
	MOVE_REG_INTO_REG_POINTER_64,

	// MOVE_MEM_INTO_MEM,

	MOVE_FRAME_OFFSET_8_INTO_REG,
	MOVE_FRAME_OFFSET_16_INTO_REG,
	MOVE_FRAME_OFFSET_32_INTO_REG,
	MOVE_FRAME_OFFSET_64_INTO_REG,

	MOVE_REG_INTO_FRAME_OFFSET_8,
	MOVE_REG_INTO_FRAME_OFFSET_16,
	MOVE_REG_INTO_FRAME_OFFSET_32,
	MOVE_REG_INTO_FRAME_OFFSET_64,

	MOVE_STACK_TOP_OFFSET_8_INTO_REG,
	MOVE_STACK_TOP_OFFSET_16_INTO_REG,
	MOVE_STACK_TOP_OFFSET_32_INTO_REG,
	MOVE_STACK_TOP_OFFSET_64_INTO_REG,

	MOVE_REG_INTO_STACK_TOP_OFFSET_8,
	MOVE_REG_INTO_STACK_TOP_OFFSET_16,
	MOVE_REG_INTO_STACK_TOP_OFFSET_32,
	MOVE_REG_INTO_STACK_TOP_OFFSET_64,

	MOVE_STACK_TOP_ADDRESS_INTO_REG,


	// Mathematical operations


	ADD_8_INTO_REG,
	ADD_16_INTO_REG,
	ADD_32_INTO_REG,
	ADD_64_INTO_REG,

	ADD_REG_INTO_REG,


	SUBTRACT_8_FROM_REG,
	SUBTRACT_16_FROM_REG,
	SUBTRACT_32_FROM_REG,
	SUBTRACT_64_FROM_REG,

	SUBTRACT_REG_FROM_REG,


	MULTIPLY_8_INTO_REG,
	MULTIPLY_16_INTO_REG,
	MULTIPLY_32_INTO_REG,
	MULTIPLY_64_INTO_REG,

	MULTIPLY_REG_INTO_REG,


	DIVIDE_8_FROM_REG,
	DIVIDE_16_FROM_REG,
	DIVIDE_32_FROM_REG,
	DIVIDE_64_FROM_REG,

	DIVIDE_REG_FROM_REG,


	TAKE_MODULO_8_OF_REG,
	TAKE_MODULO_16_OF_REG,
	TAKE_MODULO_32_OF_REG,
	TAKE_MODULO_64_OF_REG,

	TAKE_MODULO_REG_OF_REG,


	AND_8_INTO_REG,
	AND_16_INTO_REG,
	AND_32_INTO_REG,
	AND_64_INTO_REG,

	AND_REG_INTO_REG,


	OR_8_INTO_REG,
	OR_16_INTO_REG,
	OR_32_INTO_REG,
	OR_64_INTO_REG,

	OR_REG_INTO_REG,


	XOR_8_INTO_REG,
	XOR_16_INTO_REG,
	XOR_32_INTO_REG,
	XOR_64_INTO_REG,

	XOR_REG_INTO_REG,


	LEFT_SHIFT_REG_BY_8,
	LEFT_SHIFT_REG_BY_REG,

	RIGHT_SHIFT_REG_BY_8,
	RIGHT_SHIFT_REG_BY_REG,


	INCREMENT_REG,

	DECREMENT_REG,

	NOT_REG,


	// Comparison and branching


	COMPARE_8_TO_REG,
	COMPARE_16_TO_REG,
	COMPARE_32_TO_REG,
	COMPARE_64_TO_REG,

	COMPARE_REG_TO_8,
	COMPARE_REG_TO_16,
	COMPARE_REG_TO_32,
	COMPARE_REG_TO_64,

	COMPARE_REG_TO_REG,


	SET_REG_IF_GREATER,
	SET_REG_IF_GREATER_OR_EQUAL,

	SET_REG_IF_LESS,
	SET_REG_IF_LESS_OR_EQUAL,

	SET_REG_IF_EQUAL,
	SET_REG_IF_NOT_EQUAL,


	JUMP,

	JUMP_IF_GREATER,
	JUMP_IF_GREATER_OR_EQUAL,

	JUMP_IF_LESS,
	JUMP_IF_LESS_OR_EQUAL,

	JUMP_IF_EQUAL,
	JUMP_IF_NOT_EQUAL,


	// Pushing to and popping from stack


	PUSH_8,
	PUSH_16,
	PUSH_32,
	PUSH_64,

	PUSH_REG_8,
	PUSH_REG_16,
	PUSH_REG_32,
	PUSH_REG_64,


	POP_8_INTO_REG,
	POP_16_INTO_REG,
	POP_32_INTO_REG,
	POP_64_INTO_REG,


	// Functions


	CALL,
	RETURN,


	ALLOCATE_STACK,
	DEALLOCATE_STACK,


	// For debugging purposes


	COMMENT,
	LABEL,

	// System calls

	PRINT_CHAR
};

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
		default: return "UNDEFINED";
	}
}

enum ArgumentType : uint8_t {
	REG, ADDR,
	LIT_8, LIT_16, LIT_32, LIT_64,
	NULL_TERMINATED_STRING
};

vector<ArgumentType> instruction_arg_types(Instruction instruction)
{
	switch (instruction) {
		case MOVE_8_INTO_REG: return { LIT_8, REG };
		case MOVE_16_INTO_REG: return { LIT_16, REG };
		case MOVE_32_INTO_REG: return { LIT_32, REG };
		case MOVE_64_INTO_REG: return { LIT_64, REG };
		case MOVE_8_INTO_MEM: return { LIT_8, ADDR };
		case MOVE_16_INTO_MEM: return { LIT_16, ADDR };
		case MOVE_32_INTO_MEM: return { LIT_32, ADDR };
		case MOVE_64_INTO_MEM: return { LIT_64, ADDR };
		case MOVE_REG_INTO_REG: return { REG, REG };
		case MOVE_REG_INTO_MEM_8: return { REG, ADDR };
		case MOVE_REG_INTO_MEM_16: return { REG, ADDR };
		case MOVE_REG_INTO_MEM_32: return { REG, ADDR };
		case MOVE_REG_INTO_MEM_64: return { REG, ADDR };
		case MOVE_MEM_8_INTO_REG: return { ADDR, REG };
		case MOVE_MEM_16_INTO_REG: return { ADDR, REG };
		case MOVE_MEM_32_INTO_REG: return { ADDR, REG };
		case MOVE_MEM_64_INTO_REG: return { ADDR, REG };
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
		case JUMP: return { ADDR };
		case JUMP_IF_GREATER: return { ADDR };
		case JUMP_IF_GREATER_OR_EQUAL: return { ADDR };
		case JUMP_IF_LESS: return { ADDR };
		case JUMP_IF_LESS_OR_EQUAL: return { ADDR };
		case JUMP_IF_EQUAL: return { ADDR };
		case JUMP_IF_NOT_EQUAL: return { ADDR };
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
		case CALL: return { ADDR };
		case RETURN: return {};
		case ALLOCATE_STACK: return { LIT_64 };
		case DEALLOCATE_STACK: return { LIT_64 };
		case COMMENT: return { NULL_TERMINATED_STRING };
		case LABEL: return { NULL_TERMINATED_STRING };
		case PRINT_CHAR: return { REG };
		default: return {};
	}
}

#endif