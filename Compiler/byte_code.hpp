#ifndef TEA_BYTE_CODE_HEADER
#define TEA_BYTE_CODE_HEADER

#include <bits/stdc++.h>

using namespace std;

enum Instruction {
	PUSH_8,
	PUSH_16,
	PUSH_32,
	PUSH_64,
	PUSH_REG,
	POP_8,
	POP_16,
	POP_32,
	POP_64,
	POP_REG,
	JUMP,
	// MOVE_REG_REG,
	// MOVE_REG_MEM,
	// MOVE_MEM_REG,
	// MOVE_MEM_MEM,
	// CALL,
	// ADD,
	// SUBTRACT
};

#endif