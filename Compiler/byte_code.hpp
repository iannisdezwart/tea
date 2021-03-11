#ifndef TEA_BYTE_CODE_HEADER
#define TEA_BYTE_CODE_HEADER

#include <bits/stdc++.h>

using namespace std;

enum Instruction {
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

	MOVE_REG_INTO_MEM,

	MOVE_MEM_INTO_REG,

	// MOVE_MEM_INTO_MEM,

	MOVE_FRAME_OFFSET_8_INTO_REG,
	MOVE_FRAME_OFFSET_16_INTO_REG,
	MOVE_FRAME_OFFSET_32_INTO_REG,
	MOVE_FRAME_OFFSET_64_INTO_REG,

	MOVE_REG_INTO_FRAME_OFFSET,


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


	JUMP,

	JUMP_IF_GREATER,
	JUMP_IF_GREATER_OR_EQUAL,

	JUMP_IF_LESS,
	JUMP_IF_LESS_OR_EQUAL,

	JUMP_IF_EQUAL,


	// Pushing to and popping from stack


	PUSH_8,
	PUSH_16,
	PUSH_32,
	PUSH_64,

	PUSH_REG,


	POP_8_INTO_REG,
	POP_16_INTO_REG,
	POP_32_INTO_REG,
	POP_64_INTO_REG,


	// Functions


	CALL,
	RETURN,


	// For debugging purposes


	LOG_REG
};

#endif