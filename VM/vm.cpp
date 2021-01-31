#ifndef TEA_VM_HEADER
#define TEA_VM_HEADER

#include <bits/stdc++.h>

// #define CPU_DUMP_DEBUG
#include "cpu.hpp"
#include "memory.hpp"
#include "../Compiler/byte_code.hpp"

using namespace std;

int main()
{
	MemoryBuilder memory_builder;

	// memory_builder.push<uint16_t>(PUSH_32);
	// memory_builder.push<uint32_t>(123456);
	// memory_builder.push<uint16_t>(PUSH_8);
	// memory_builder.push<uint8_t>(222);
	// memory_builder.push<uint16_t>(POP_8);
	// memory_builder.push<uint16_t>(POP_32);

	// memory_builder.push<uint16_t>(JUMP);
	// memory_builder.push<uint64_t>(0);

	// R_1 = 0

	memory_builder.push<uint16_t>(MOVE_LIT_INTO_REG);
	memory_builder.push<uint8_t>(R_1_ID);
	memory_builder.push<uint64_t>(0);

	// R_2 = 1

	memory_builder.push<uint16_t>(MOVE_LIT_INTO_REG);
	memory_builder.push<uint8_t>(R_2_ID);
	memory_builder.push<uint64_t>(1);

	// R_3 = R_1

	size_t label = memory_builder.push<uint16_t>(MOVE_REG_INTO_REG);
	memory_builder.push<uint8_t>(R_1_ID);
	memory_builder.push<uint8_t>(R_3_ID);

	// R_3 += R_2

	memory_builder.push<uint16_t>(ADD_REG_INTO_REG);
	memory_builder.push<uint8_t>(R_2_ID);
	memory_builder.push<uint8_t>(R_3_ID);

	// R_1 = R_2

	memory_builder.push<uint16_t>(MOVE_REG_INTO_REG);
	memory_builder.push<uint8_t>(R_2_ID);
	memory_builder.push<uint8_t>(R_1_ID);

	// R_2 = R_3

	memory_builder.push<uint16_t>(MOVE_REG_INTO_REG);
	memory_builder.push<uint8_t>(R_3_ID);
	memory_builder.push<uint8_t>(R_2_ID);

	// Push so we will terminate after a while because of a segfault

	memory_builder.push<uint16_t>(PUSH_64);
	memory_builder.push<uint64_t>(0);

	// Loop

	memory_builder.push<uint16_t>(JUMP);
	memory_builder.push<uint64_t>(label);

	CPU cpu(memory_builder, 100);

	cpu.run();
	cpu.dump_stack();
	cpu.dump_registers();
}

#endif