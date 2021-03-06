#ifndef TEA_DISASSEMBLER_HEADER
#define TEA_DISASSEMBLER_HEADER

#include <bits/stdc++.h>

#include "file-reader.hpp"
#include "../ansi.hpp"
#include "../Assembler/byte_code.hpp"
#include "../VM/memory-device.hpp"
#include "../VM/cpu.hpp"

using namespace std;

class Disassembler {
	public:
		FileReader file_reader;
		FILE *file_out;
		bool first_arg;

		Disassembler(FILE *file_in, FILE *file_out)
			: file_reader(file_in), file_out(file_out), first_arg(true) {}

		void print_instruction(const char *instruction)
		{
			// Print address in green

			fprintf(file_out, ANSI_GREEN "0x" ANSI_BRIGHT_GREEN "%04lx" ANSI_RESET "    ",
				PROGRAM_START + file_reader.read_bytes - 18);

			// Print instruction in orange

			fprintf(file_out, ANSI_YELLOW "%-33s" ANSI_RESET, instruction);
		}

		void print_arg()
		{
			if (first_arg) {
				first_arg = false;
				fprintf(file_out, " ");
			} else {
				fprintf(file_out, ", ");
			}
		}

		void print_arg_reg(uint8_t reg_id)
		{
			print_arg();
			fprintf(file_out, ANSI_CYAN "%s" ANSI_RESET, CPU::reg_to_str(reg_id));
		}

		void print_arg_literal_number(uint64_t num)
		{
			print_arg();
			fprintf(file_out, ANSI_YELLOW "%ld" ANSI_RESET, num);
		}

		void print_arg_address(uint64_t address)
		{
			print_arg();
			fprintf(file_out, ANSI_GREEN "0x" ANSI_BRIGHT_GREEN "%lx" ANSI_RESET, address);
		}

		void print_arg_null_terminated_string(char *str)
		{
			print_arg();
			fprintf(file_out, ANSI_BRIGHT_BLACK "/* %s */", str);
		}

		void end_args()
		{
			fprintf(file_out, "\n");
			first_arg = true;
		}

		void disassemble()
		{
			uint64_t static_data_size = file_reader.read<uint64_t>();
			uint64_t program_size = file_reader.read<uint64_t>();

			// Print static data

			fprintf(file_out, "Static data (size = %lu)\n\n", static_data_size);

			for (size_t i = 0; i < static_data_size; i++) {
				uint8_t byte = file_reader.read<uint8_t>();

				fprintf(file_out, "0x%04lx    0x%02hhx    %03hhu    '%c'\n",
					PROGRAM_START + i, byte, byte, byte);
			}

			// Print program

			fprintf(file_out, "\nProgram (size = %lu)\n\n", program_size);

			Instruction instruction;

			while (
				(instruction = (Instruction) file_reader.read<uint16_t>())
				!= (1 << 16) - 1
			) {
				const char * instruction_str = instruction_to_str(instruction);
				vector<ArgumentType> args = instruction_arg_types(instruction);

				print_instruction(instruction_str);

				for (ArgumentType arg : args) {
					switch (arg) {
						case REG:
							print_arg_reg(file_reader.read<uint8_t>());
							break;

						case ADDR:
							print_arg_address(file_reader.read<uint64_t>());
							break;

						case LIT_8:
							print_arg_literal_number(file_reader.read<uint8_t>());
							break;

						case LIT_16:
							print_arg_literal_number(file_reader.read<uint16_t>());
							break;

						case LIT_32:
							print_arg_literal_number(file_reader.read<uint32_t>());
							break;

						case LIT_64:
							print_arg_literal_number(file_reader.read<uint64_t>());
							break;

						case NULL_TERMINATED_STRING:
						{
							vector<char> str;
							char c;

							do {
								c = file_reader.read<char>();
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
		}
};

#endif