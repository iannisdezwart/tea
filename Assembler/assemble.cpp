#include <bits/stdc++.h>

#include "../VM/cpu.hpp"
#include "../VM/memory.hpp"
#include "assembler.hpp"

using namespace std;

Assembler fibonacci_demo()
{
	Assembler assembler;

	// r_0 = 0, r_1 = 1

	assembler.move_8_into_reg(0, R_0_ID);
	assembler.move_8_into_reg(1, R_1_ID);

	assembler.add_label("loop");

	assembler.log_reg(R_0_ID);

	// r_2 = r_0 + r_1

	assembler.move_reg_into_reg(R_0_ID, R_2_ID);
	assembler.add_reg_into_reg(R_1_ID, R_2_ID);

	// r_0 = r_1, r_1 = r_2

	assembler.move_reg_into_reg(R_1_ID, R_0_ID);
	assembler.move_reg_into_reg(R_2_ID, R_1_ID);

	assembler.compare_reg_to_64(R_0_ID, 200);
	assembler.jump_if_less("loop");

	return assembler;
}

Assembler stack_demo()
{
	Assembler assembler;

	// Push arguments for the add function
	// We will add 5 and 6

	assembler.push_64(5); // Argument 1
	assembler.push_64(6); // Argument 2
	assembler.push_64(16); // Size of arguments in bytes

	// Call the add function, log its return value, and exit the program

	assembler.call("add_two_uint64s");
	assembler.log_reg(R_ACCUMULATOR_ID);
	assembler.jump("exit");

	// Where the add function lives in memory

	assembler.add_label("add_two_uint64s");

	// Move the parameters into registers

	assembler.move_frame_offset_64_into_reg(CPU::stack_frame_size + 8, R_ACCUMULATOR_ID);
	assembler.move_frame_offset_64_into_reg(CPU::stack_frame_size + 16, R_0_ID);

	// Add parameter 2 into parameter 1

	assembler.add_reg_into_reg(R_0_ID, R_ACCUMULATOR_ID);

	// The accumulator is set to the return value, now we can return

	assembler.return_();

	// Exit label

	assembler.add_label("exit");

	return assembler;
}

Assembler static_data_demo()
{
	Assembler assembler;

	// Store some messages into the static data

	const char *a = "Hello, World!\n";
	StaticData hello_world = assembler.add_static_data((const uint8_t *) a, strlen(a));

	const char *b = "Bye, World!\n";
	StaticData bye_world = assembler.add_static_data((const uint8_t *) b, strlen(b));

	const char *c = "It wurk!\n";
	StaticData it_wurk = assembler.add_static_data((const uint8_t *) c, strlen(c));

	// Print the first message

	assembler.move_32_into_reg(hello_world.offset, R_0_ID);
	assembler.push_reg(R_0_ID);
	assembler.push_64(hello_world.size);
	assembler.push_64(16);
	assembler.call("print");

	// Print the second message

	assembler.move_32_into_reg(bye_world.offset, R_0_ID);
	assembler.push_reg(R_0_ID);
	assembler.push_64(bye_world.size);
	assembler.push_64(16);
	assembler.call("print");

	// Print the third message

	assembler.move_32_into_reg(it_wurk.offset, R_0_ID);
	assembler.push_reg(R_0_ID);
	assembler.push_64(it_wurk.size);
	assembler.push_64(16);
	assembler.call("print");

	// Exit the program

	assembler.jump("exit");



	// Printing function

	assembler.add_label("print");

	// Move the start pointer to the string into R_0

	assembler.move_frame_offset_64_into_reg(CPU::stack_frame_size + 16, R_0_ID);

	// Move the size parameter into R_1

	assembler.move_frame_offset_64_into_reg(CPU::stack_frame_size + 8, R_1_ID);

	// Add the start pointer to the size, so we get the end pointer

	assembler.add_reg_into_reg(R_0_ID, R_1_ID);

	assembler.add_label("print_loop");

	// Get the byte at the running pointer and increment the pointer

	assembler.move_reg_pointer_8_into_reg(R_0_ID, R_2_ID);
	assembler.increment_reg(R_0_ID);

	// Print the character

	assembler.print_char_from_reg(R_2_ID);

	// Loop if the running pointer is less than the end pointer

	assembler.compare_reg_to_reg(R_0_ID, R_1_ID);
	assembler.jump_if_less("print_loop");

	// Return

	assembler.return_();



	// Exit point for the program

	assembler.add_label("exit");

	return assembler;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage: ./assemble output_file_name.teax\n");
		exit(1);
	}

	const char *file_path = argv[1];

	Assembler assembler = static_data_demo();
	Buffer executable = assembler.assemble();
	executable.write_to_file(file_path);
}