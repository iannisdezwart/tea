#ifndef TEA_DEBUGGER_INSTRUCTION_LISTER_HEADER
#define TEA_DEBUGGER_INSTRUCTION_LISTER_HEADER

#include <bits/stdc++.h>
#include "../ansi.hpp"
#include "../VM/cpu.hpp"
#include "../VM/memory.hpp"
#include "util.hpp"

using namespace std;

class InstructionLister {
	public:
		memory::Reader& reader;
		bool first_arg;

		InstructionLister(memory::Reader& reader)
			: reader(reader), first_arg(true) {}

		void print_instruction(const char *instruction, const PtrSet& breakpoints)
		{
			uint8_t *address = reader.addr - 2;

			// Print the address in red if a breakpoint was set to it

			if (breakpoints.count(address)) {
				printf(ANSI_RED ANSI_BOLD "0x" ANSI_BRIGHT_RED "%04lx" ANSI_RESET "    ",
					(uint64_t) address);
			}

			// Print address in green otherwise

			else {
				printf(ANSI_GREEN ANSI_BOLD "0x" ANSI_BRIGHT_GREEN "%04lx" ANSI_RESET "    ",
					(uint64_t) address);
			}

			// Print instruction in orange

			printf(ANSI_YELLOW ANSI_ITALIC "%-33s" ANSI_RESET, instruction);
		}

		void print_arg()
		{
			if (first_arg) {
				first_arg = false;
				printf(" ");
			} else {
				printf(", ");
			}
		}

		void print_arg_reg(uint8_t reg_id)
		{
			print_arg();
			printf(ANSI_CYAN ANSI_ITALIC "%s" ANSI_RESET, CPU::reg_to_str(reg_id));
		}

		void print_arg_literal_number(uint64_t num)
		{
			print_arg();
			printf(ANSI_YELLOW "%ld" ANSI_RESET, num);
		}

		void print_arg_address(uint64_t address)
		{
			print_arg();
			printf(ANSI_GREEN ANSI_BOLD "0x" ANSI_BRIGHT_GREEN "%lx" ANSI_RESET, address);
		}

		void print_arg_null_terminated_string(char *str)
		{
			print_arg();
			printf(ANSI_BRIGHT_BLACK "/* %s */", str);
		}

		void end_args()
		{
			printf("\n");
			first_arg = true;
		}

		void disassemble_one(const PtrSet& breakpoints)
		{
			// Read the next instruction and print it

			Instruction instruction = (Instruction) reader.read<uint16_t>();
			const char * instruction_str = instruction_to_str(instruction);
			vector<ArgumentType> args = instruction_arg_types(instruction);

			print_instruction(instruction_str, breakpoints);

			// Print the arguments

			for (ArgumentType arg : args) {
				switch (arg) {
					case REG:
						print_arg_reg(reader.read<uint8_t>());
						break;

					case ADDR:
						print_arg_address(reader.read<uint64_t>());
						break;

					case LIT_8:
						print_arg_literal_number(reader.read<uint8_t>());
						break;

					case LIT_16:
						print_arg_literal_number(reader.read<uint16_t>());
						break;

					case LIT_32:
						print_arg_literal_number(reader.read<uint32_t>());
						break;

					case LIT_64:
						print_arg_literal_number(reader.read<uint64_t>());
						break;

					case NULL_TERMINATED_STRING:
					{
						vector<char> str;
						char c;

						do {
							c = reader.read<char>();
							str.push_back(c);
						} while (c != '\0');

						print_arg_null_terminated_string(str.data());
						break;
					}

					default:
						fprintf(stderr, "I think I messed up the code again ;-;");
						abort();
				}
			}

			end_args();
		}

		void disassemble(size_t num_of_instructions, const PtrSet& breakpoints)
		{
			for (size_t i = 0; i < num_of_instructions; i++) {
				// if (!reader.is_safe()) break;
				disassemble_one(breakpoints);
			}
		}

		void disassemble_all(uint8_t *top, const PtrSet& breakpoints)
		{
			while (reader.addr < top) {
				// if (!reader.is_safe()) break;
				disassemble_one(breakpoints);
			}
		}
};


#endif