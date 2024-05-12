#ifndef TEA_BYTE_CODE_HEADER
#define TEA_BYTE_CODE_HEADER

#include <cstdint>
#include <vector>

/**
 * @brief An enum containing all valid opcodes.
 */
enum Instruction : uint16_t
{
	// ===========================
	// === Memory instructions ===
	// ===========================

	// Moves a 64-bit literal into a register.
	MOVE_LIT,

	// Moves the contents of a 64-bit register into a 64-bit register.
	MOVE,

	// Dereference pointer stored register into register.
	LOAD_PTR_8,
	LOAD_PTR_16,
	LOAD_PTR_32,
	LOAD_PTR_64,

	// Store register value into dereferenced pointer.
	STORE_PTR_8,
	STORE_PTR_16,
	STORE_PTR_32,
	STORE_PTR_64,

	// Memory copy register pointer into register pointer.
	MEM_COPY,

	// ===============================
	// === Mathematical operations ===
	// ===============================

	// Adds the contents of a register to a register.
	ADD_INT_8,
	ADD_INT_16,
	ADD_INT_32,
	ADD_INT_64,
	ADD_FLT_32,
	ADD_FLT_64,

	// Subtracts the contents of a register from a register.
	SUB_INT_8,
	SUB_INT_16,
	SUB_INT_32,
	SUB_INT_64,
	SUB_FLT_32,
	SUB_FLT_64,

	// Multiplies the contents of a register with a register.
	MUL_INT_8,
	MUL_INT_16,
	MUL_INT_32,
	MUL_INT_64,
	MUL_FLT_32,
	MUL_FLT_64,

	// Divides the contents of a register by a register.
	DIV_INT_8,
	DIV_INT_16,
	DIV_INT_32,
	DIV_INT_64,
	DIV_FLT_32,
	DIV_FLT_64,

	// Takes the modulo of a register by a register.
	MOD_INT_8,
	MOD_INT_16,
	MOD_INT_32,
	MOD_INT_64,

	// Performs bitwise AND on a register with a register.
	AND_INT_8,
	AND_INT_16,
	AND_INT_32,
	AND_INT_64,

	// Performs bitwise OR on a register with a register.
	OR_INT_8,
	OR_INT_16,
	OR_INT_32,
	OR_INT_64,

	// Performs bitwise XOR on a register with a register.
	XOR_INT_8,
	XOR_INT_16,
	XOR_INT_32,
	XOR_INT_64,

	// Left shifts a register by a register.
	SHL_INT_8,
	SHL_INT_16,
	SHL_INT_32,
	SHL_INT_64,

	// Right shifts a register by a register.
	SHR_INT_8,
	SHR_INT_16,
	SHR_INT_32,
	SHR_INT_64,

	// Increments a register by one.
	INC_INT_8,
	INC_INT_16,
	INC_INT_32,
	INC_INT_64,

	// Decrements a register by one.
	DEC_INT_8,
	DEC_INT_16,
	DEC_INT_32,
	DEC_INT_64,

	// Flips the bits in a register.
	NEG_INT_8,
	NEG_INT_16,
	NEG_INT_32,
	NEG_INT_64,

	// ================================
	// === Cast and type operations ===
	// ================================

	CAST_INT_TO_FLT_32,
	CAST_INT_TO_FLT_64,
	CAST_FLT_32_TO_INT,
	CAST_FLT_64_TO_INT,

	// ================================
	// === Comparison and branching ===
	// ================================

	// Compare register with register.
	CMP_INT_8,
	CMP_INT_8_U,
	CMP_INT_16,
	CMP_INT_16_U,
	CMP_INT_32,
	CMP_INT_32_U,
	CMP_INT_64,
	CMP_INT_64_U,
	CMP_FLT_32,
	CMP_FLT_64,

	// Sets a register based on a comparison.
	SET_IF_GT,
	SET_IF_GEQ,
	SET_IF_LT,
	SET_IF_LEQ,
	SET_IF_EQ,
	SET_IF_NEQ,

	// Unconditional jump.
	JUMP,

	// Conditional jumps.
	JUMP_IF_GT,
	JUMP_IF_GEQ,
	JUMP_IF_LT,
	JUMP_IF_LEQ,
	JUMP_IF_EQ,
	JUMP_IF_NEQ,

	// ========================
	// === Stack operations ===
	// ========================

	// Pushing to the stack.
	PUSH_REG_8,
	PUSH_REG_16,
	PUSH_REG_32,
	PUSH_REG_64,

	// Popping from the stack.
	POP_8_INTO_REG,
	POP_16_INTO_REG,
	POP_32_INTO_REG,
	POP_64_INTO_REG,

	// ======================
	// === Function calls ===
	// ======================

	// Calls a function.
	CALL,

	// Returns from a function.
	RETURN,

	// Allocate a number of bytes on the stack.
	ALLOCATE_STACK,

	// Deallocate a number of bytes on the stack.
	DEALLOCATE_STACK,

	// ======================
	// === Miscellaneous ===
	// ======================

	// A comment that is ignored by the VM.
	COMMENT,

	// A label that is used for debugging.
	LABEL,

	// ======================
	// === I/O operations ===
	// ======================

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
const char *
instruction_to_str(Instruction instruction)
{
	switch (instruction)
	{
	case MOVE_LIT:
		return "MOVE_LIT";
	case MOVE:
		return "MOVE";
	case LOAD_PTR_8:
		return "LOAD_PTR_8";
	case LOAD_PTR_16:
		return "LOAD_PTR_16";
	case LOAD_PTR_32:
		return "LOAD_PTR_32";
	case LOAD_PTR_64:
		return "LOAD_PTR_64";
	case STORE_PTR_8:
		return "STORE_PTR_8";
	case STORE_PTR_16:
		return "STORE_PTR_16";
	case STORE_PTR_32:
		return "STORE_PTR_32";
	case STORE_PTR_64:
		return "STORE_PTR_64";
	case MEM_COPY:
		return "MEM_COPY";
	case ADD_INT_8:
		return "ADD_INT_8";
	case ADD_INT_16:
		return "ADD_INT_16";
	case ADD_INT_32:
		return "ADD_INT_32";
	case ADD_INT_64:
		return "ADD_INT_64";
	case ADD_FLT_32:
		return "ADD_FLT_32";
	case ADD_FLT_64:
		return "ADD_FLT_64";
	case SUB_INT_8:
		return "SUB_INT_8";
	case SUB_INT_16:
		return "SUB_INT_16";
	case SUB_INT_32:
		return "SUB_INT_32";
	case SUB_INT_64:
		return "SUB_INT_64";
	case SUB_FLT_32:
		return "SUB_FLT_32";
	case SUB_FLT_64:
		return "SUB_FLT_64";
	case MUL_INT_8:
		return "MUL_INT_8";
	case MUL_INT_16:
		return "MUL_INT_16";
	case MUL_INT_32:
		return "MUL_INT_32";
	case MUL_INT_64:
		return "MUL_INT_64";
	case MUL_FLT_32:
		return "MUL_FLT_32";
	case MUL_FLT_64:
		return "MUL_FLT_64";
	case DIV_INT_8:
		return "DIV_INT_8";
	case DIV_INT_16:
		return "DIV_INT_16";
	case DIV_INT_32:
		return "DIV_INT_32";
	case DIV_INT_64:
		return "DIV_INT_64";
	case DIV_FLT_32:
		return "DIV_FLT_32";
	case DIV_FLT_64:
		return "DIV_FLT_64";
	case MOD_INT_8:
		return "MOD_INT_8";
	case MOD_INT_16:
		return "MOD_INT_16";
	case MOD_INT_32:
		return "MOD_INT_32";
	case MOD_INT_64:
		return "MOD_INT_64";
	case AND_INT_8:
		return "AND_INT_8";
	case AND_INT_16:
		return "AND_INT_16";
	case AND_INT_32:
		return "AND_INT_32";
	case AND_INT_64:
		return "AND_INT_64";
	case OR_INT_8:
		return "OR_INT_8";
	case OR_INT_16:
		return "OR_INT_16";
	case OR_INT_32:
		return "OR_INT_32";
	case OR_INT_64:
		return "OR_INT_64";
	case XOR_INT_8:
		return "XOR_INT_8";
	case XOR_INT_16:
		return "XOR_INT_16";
	case XOR_INT_32:
		return "XOR_INT_32";
	case XOR_INT_64:
		return "XOR_INT_64";
	case SHL_INT_8:
		return "SHL_INT_8";
	case SHL_INT_16:
		return "SHL_INT_16";
	case SHL_INT_32:
		return "SHL_INT_32";
	case SHL_INT_64:
		return "SHL_INT_64";
	case SHR_INT_8:
		return "SHR_INT_8";
	case SHR_INT_16:
		return "SHR_INT_16";
	case SHR_INT_32:
		return "SHR_INT_32";
	case SHR_INT_64:
		return "SHR_INT_64";
	case INC_INT_8:
		return "INC_INT_8";
	case INC_INT_16:
		return "INC_INT_16";
	case INC_INT_32:
		return "INC_INT_32";
	case INC_INT_64:
		return "INC_INT_64";
	case DEC_INT_8:
		return "DEC_INT_8";
	case DEC_INT_16:
		return "DEC_INT_16";
	case DEC_INT_32:
		return "DEC_INT_32";
	case DEC_INT_64:
		return "DEC_INT_64";
	case NEG_INT_8:
		return "NEG_INT_8";
	case NEG_INT_16:
		return "NEG_INT_16";
	case NEG_INT_32:
		return "NEG_INT_32";
	case NEG_INT_64:
		return "NEG_INT_64";
	case CAST_INT_TO_FLT_32:
		return "CAST_INT_TO_FLT_32";
	case CAST_INT_TO_FLT_64:
		return "CAST_INT_TO_FLT_64";
	case CAST_FLT_32_TO_INT:
		return "CAST_FLT_32_TO_INT";
	case CAST_FLT_64_TO_INT:
		return "CAST_FLT_64_TO_INT";
	case CMP_INT_8:
		return "CMP_INT_8";
	case CMP_INT_8_U:
		return "CMP_INT_8_U";
	case CMP_INT_16:
		return "CMP_INT_16";
	case CMP_INT_16_U:
		return "CMP_INT_16_U";
	case CMP_INT_32:
		return "CMP_INT_32";
	case CMP_INT_32_U:
		return "CMP_INT_32_U";
	case CMP_INT_64:
		return "CMP_INT_64";
	case CMP_INT_64_U:
		return "CMP_INT_64_U";
	case CMP_FLT_32:
		return "CMP_FLT_32";
	case CMP_FLT_64:
		return "CMP_FLT_64";
	case SET_IF_GT:
		return "SET_IF_GT";
	case SET_IF_GEQ:
		return "SET_IF_GEQ";
	case SET_IF_LT:
		return "SET_IF_LT";
	case SET_IF_LEQ:
		return "SET_IF_LEQ";
	case SET_IF_EQ:
		return "SET_IF_EQ";
	case SET_IF_NEQ:
		return "SET_IF_NEQ";
	case JUMP:
		return "JUMP";
	case JUMP_IF_GT:
		return "JUMP_IF_GT";
	case JUMP_IF_GEQ:
		return "JUMP_IF_GEQ";
	case JUMP_IF_LT:
		return "JUMP_IF_LT";
	case JUMP_IF_LEQ:
		return "JUMP_IF_LEQ";
	case JUMP_IF_EQ:
		return "JUMP_IF_EQ";
	case JUMP_IF_NEQ:
		return "JUMP_IF_NEQ";
	case PUSH_REG_8:
		return "PUSH_REG_8";
	case PUSH_REG_16:
		return "PUSH_REG_16";
	case PUSH_REG_32:
		return "PUSH_REG_32";
	case PUSH_REG_64:
		return "PUSH_REG_64";
	case POP_8_INTO_REG:
		return "POP_8_INTO_REG";
	case POP_16_INTO_REG:
		return "POP_16_INTO_REG";
	case POP_32_INTO_REG:
		return "POP_32_INTO_REG";
	case POP_64_INTO_REG:
		return "POP_64_INTO_REG";
	case CALL:
		return "CALL";
	case RETURN:
		return "RETURN";
	case ALLOCATE_STACK:
		return "ALLOCATE_STACK";
	case DEALLOCATE_STACK:
		return "DEALLOCATE_STACK";
	case COMMENT:
		return "COMMENT";
	case LABEL:
		return "LABEL";
	case PRINT_CHAR:
		return "PRINT_CHAR";
	case GET_CHAR:
		return "GET_CHAR";
	default:
		return "UNDEFINED";
	}
}

/**
 * @brief An enum of all valid argument types.
 */
enum ArgumentType : uint8_t
{
	REG,
	REL_ADDR,
	LIT_8,
	LIT_16,
	LIT_32,
	LIT_64,
	NULL_TERMINATED_STRING
};

/**
 * @brief Returns the allowed argument types for a given instruction.
 * @param instruction The instruction to get the argument types for.
 */
std::vector<ArgumentType>
instruction_arg_types(Instruction instruction)
{
	switch (instruction)
	{
	case MOVE_LIT:
		return { LIT_64, REG };
	case MOVE:
	case LOAD_PTR_8:
	case LOAD_PTR_16:
	case LOAD_PTR_32:
	case LOAD_PTR_64:
	case STORE_PTR_8:
	case STORE_PTR_16:
	case STORE_PTR_32:
	case STORE_PTR_64:
		return { REG, REG };
	case MEM_COPY:
		return { REG, REG, LIT_64 };
	case ADD_INT_8:
	case ADD_INT_16:
	case ADD_INT_32:
	case ADD_INT_64:
	case ADD_FLT_32:
	case ADD_FLT_64:
	case SUB_INT_8:
	case SUB_INT_16:
	case SUB_INT_32:
	case SUB_INT_64:
	case SUB_FLT_32:
	case SUB_FLT_64:
	case MUL_INT_8:
	case MUL_INT_16:
	case MUL_INT_32:
	case MUL_INT_64:
	case MUL_FLT_32:
	case MUL_FLT_64:
	case DIV_INT_8:
	case DIV_INT_16:
	case DIV_INT_32:
	case DIV_INT_64:
	case DIV_FLT_32:
	case DIV_FLT_64:
	case MOD_INT_8:
	case MOD_INT_16:
	case MOD_INT_32:
	case MOD_INT_64:
	case AND_INT_8:
	case AND_INT_16:
	case AND_INT_32:
	case AND_INT_64:
	case OR_INT_8:
	case OR_INT_16:
	case OR_INT_32:
	case OR_INT_64:
	case XOR_INT_8:
	case XOR_INT_16:
	case XOR_INT_32:
	case XOR_INT_64:
	case SHL_INT_8:
	case SHL_INT_16:
	case SHL_INT_32:
	case SHL_INT_64:
	case SHR_INT_8:
	case SHR_INT_16:
	case SHR_INT_32:
	case SHR_INT_64:
		return { REG, REG };
	case INC_INT_8:
	case INC_INT_16:
	case INC_INT_32:
	case INC_INT_64:
	case DEC_INT_8:
	case DEC_INT_16:
	case DEC_INT_32:
	case DEC_INT_64:
	case NEG_INT_8:
	case NEG_INT_16:
	case NEG_INT_32:
	case NEG_INT_64:
	case CAST_INT_TO_FLT_32:
	case CAST_INT_TO_FLT_64:
	case CAST_FLT_32_TO_INT:
	case CAST_FLT_64_TO_INT:
		return { REG };
	case CMP_INT_8:
	case CMP_INT_8_U:
	case CMP_INT_16:
	case CMP_INT_16_U:
	case CMP_INT_32:
	case CMP_INT_32_U:
	case CMP_INT_64:
	case CMP_INT_64_U:
	case CMP_FLT_32:
	case CMP_FLT_64:
		return { REG, REG };
	case SET_IF_GT:
	case SET_IF_GEQ:
	case SET_IF_LT:
	case SET_IF_LEQ:
	case SET_IF_EQ:
	case SET_IF_NEQ:
		return { REG };
	case JUMP:
	case JUMP_IF_GT:
	case JUMP_IF_GEQ:
	case JUMP_IF_LT:
	case JUMP_IF_LEQ:
	case JUMP_IF_EQ:
	case JUMP_IF_NEQ:
		return { REL_ADDR };
	case PUSH_REG_8:
	case PUSH_REG_16:
	case PUSH_REG_32:
	case PUSH_REG_64:
	case POP_8_INTO_REG:
	case POP_16_INTO_REG:
	case POP_32_INTO_REG:
	case POP_64_INTO_REG:
		return { REG };
	case CALL:
		return { REL_ADDR };
	case RETURN:
		return {};
	case ALLOCATE_STACK:
	case DEALLOCATE_STACK:
		return { LIT_64 };
	case COMMENT:
	case LABEL:
		return { NULL_TERMINATED_STRING };
	case PRINT_CHAR:
	case GET_CHAR:
		return { REG };
	default:
		return {};
	}
}

#endif