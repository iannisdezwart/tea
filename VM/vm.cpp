#ifndef TEA_VM_HEADER
#define TEA_VM_HEADER

#include <bits/stdc++.h>

#define CPU_DUMP_DEBUG
#include "cpu.hpp"
#include "memory.hpp"
#include "../Compiler/byte_code.hpp"

using namespace std;

int main()
{
	// Simple fibonacci demonstration

	ProgramBuilder program_builder;

	program_builder.move_8_into_reg(0, R_0_ID);
	program_builder.move_8_into_reg(1, R_1_ID);

	program_builder.add_label("loop");

	program_builder.move_reg_into_reg(R_0_ID, R_2_ID);
	program_builder.add_reg_into_reg(R_1_ID, R_2_ID);

	program_builder.move_reg_into_reg(R_1_ID, R_0_ID);
	program_builder.move_reg_into_reg(R_2_ID, R_1_ID);

	program_builder.compare_reg_to_64(R_0_ID, 200);
	program_builder.jump_if_less("loop");

	CPU cpu(program_builder, 12);

	cpu.run();
}

#endif