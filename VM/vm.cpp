#ifndef TEA_VM_HEADER
#define TEA_VM_HEADER

#include <bits/stdc++.h>

#define CPU_DUMP_DEBUG
#include "cpu.hpp"
#include "memory.hpp"
#include "../Compiler/byte_code.hpp"

using namespace std;

ProgramBuilder fibonacci_demo()
{
	ProgramBuilder program_builder;

	// r_0 = 0, r_1 = 1

	program_builder.move_8_into_reg(0, R_0_ID);
	program_builder.move_8_into_reg(1, R_1_ID);

	program_builder.add_label("loop");

	program_builder.log_reg(R_0_ID);

	// r_2 = r_0 + r_1

	program_builder.move_reg_into_reg(R_0_ID, R_2_ID);
	program_builder.add_reg_into_reg(R_1_ID, R_2_ID);

	// r_0 = r_1, r_1 = r_2

	program_builder.move_reg_into_reg(R_1_ID, R_0_ID);
	program_builder.move_reg_into_reg(R_2_ID, R_1_ID);

	program_builder.compare_reg_to_64(R_0_ID, 200);
	program_builder.jump_if_less("loop");

	return program_builder;
}

ProgramBuilder stack_demo()
{
	ProgramBuilder program_builder;

	const char *c = "Hello, World!";
	program_builder.add_static_data("hello_world", (const uint8_t *) c, strlen(c));

	// Push arguments for the add function
	// We will add 5 and 6

	program_builder.push_64(5); // Argument 1
	program_builder.push_64(6); // Argument 2
	program_builder.push_64(16); // Size of arguments in bytes

	// Call the add function, log its return value, and exit the program

	program_builder.call("add_two_uint64s");
	program_builder.log_reg(R_ACCUMULATOR_ID);
	program_builder.jump("exit");

	// Where the add function lives in memory

	program_builder.add_label("add_two_uint64s");

	// Move the parameters into registers

	program_builder.move_frame_offset_64_into_reg(56, R_ACCUMULATOR_ID);
	program_builder.move_frame_offset_64_into_reg(56 + 8, R_0_ID);

	// Add parameter 2 into parameter 1

	program_builder.add_reg_into_reg(R_0_ID, R_ACCUMULATOR_ID);

	// The accumulator is set to the return value, now we can return

	program_builder.return_();

	// Exit label

	program_builder.add_label("exit");

	return program_builder;
}

ProgramBuilder cooler_stack_demo()
{
	ProgramBuilder program_builder;

	// 

	return program_builder;
}

int main()
{
	ProgramBuilder program_builder = stack_demo();
	CPU cpu(program_builder, 100);
	cpu.run();
}

#endif