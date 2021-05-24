#ifndef TEA_DISASSEMBLER_HEADER
#define TEA_DISASSEMBLER_HEADER

#include <bits/stdc++.h>

#include "file-reader.hpp"
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

			fprintf(file_out, "\x1b[32m0x\x1b[92m%04lx\x1b[m    ",
				PROGRAM_START + file_reader.read_bytes - 18);

			// Print instruction in orange

			fprintf(file_out, "\x1b[33m%-33s\x1b[m", instruction);
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
			fprintf(file_out, "\x1b[36m%s\x1b[m", CPU::reg_to_str(reg_id));
		}

		void print_arg_literal_number(uint64_t num)
		{
			print_arg();
			fprintf(file_out, "\x1b[33m%ld\x1b[m", num);
		}

		void print_arg_address(uint64_t address)
		{
			print_arg();
			fprintf(file_out, "\x1b[32m0x\x1b[92m%lx\x1b[m", address);
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
				switch (instruction) {
					default:
					{
						fprintf(file_out, "Instruction %hu is undefined\n"
							"I probably messed up the code again ;-;\n", instruction);
						abort();
					}

					case MOVE_8_INTO_REG:
					{
						print_instruction("MOVE_8_INTO_REG");

						uint8_t lit = file_reader.read<uint8_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case MOVE_16_INTO_REG:
					{
						print_instruction("MOVE_16_INTO_REG");

						uint16_t lit = file_reader.read<uint16_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case MOVE_32_INTO_REG:
					{
						print_instruction("MOVE_32_INTO_REG");

						uint32_t lit = file_reader.read<uint32_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case MOVE_64_INTO_REG:
					{
						print_instruction("MOVE_64_INTO_REG");

						uint64_t lit = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case MOVE_8_INTO_MEM:
					{
						print_instruction("MOVE_8_INTO_MEM");

						uint8_t lit = file_reader.read<uint8_t>();
						uint64_t address = file_reader.read<uint64_t>();

						print_arg_literal_number(lit);
						print_arg_address(address);
						break;
					}

					case MOVE_16_INTO_MEM:
					{
						print_instruction("MOVE_16_INTO_MEM");

						uint16_t lit = file_reader.read<uint16_t>();
						uint64_t address = file_reader.read<uint64_t>();

						print_arg_literal_number(lit);
						print_arg_address(address);
						break;
					}

					case MOVE_32_INTO_MEM:
					{
						print_instruction("MOVE_32_INTO_MEM");

						uint32_t lit = file_reader.read<uint32_t>();
						uint64_t address = file_reader.read<uint64_t>();

						print_arg_literal_number(lit);
						print_arg_address(address);
						break;
					}

					case MOVE_64_INTO_MEM:
					{
						print_instruction("MOVE_64_INTO_MEM");

						uint64_t lit = file_reader.read<uint64_t>();
						uint64_t address = file_reader.read<uint64_t>();

						print_arg_literal_number(lit);
						print_arg_address(address);
						break;
					}

					case MOVE_REG_INTO_REG:
					{
						print_instruction("MOVE_REG_INTO_REG");

						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						print_arg_reg(reg_id_1);
						print_arg_reg(reg_id_2);
						break;
					}

					case MOVE_REG_INTO_MEM_8:
					{
						print_instruction("MOVE_REG_INTO_MEM_8");

						uint8_t reg_id = file_reader.read<uint8_t>();
						uint64_t address = file_reader.read<uint64_t>();

						print_arg_reg(reg_id);
						print_arg_address(address);
						break;
					}

					case MOVE_REG_INTO_MEM_16:
					{
						print_instruction("MOVE_REG_INTO_MEM_16");

						uint8_t reg_id = file_reader.read<uint8_t>();
						uint64_t address = file_reader.read<uint64_t>();

						print_arg_reg(reg_id);
						print_arg_address(address);
						break;
					}

					case MOVE_REG_INTO_MEM_32:
					{
						print_instruction("MOVE_REG_INTO_MEM_32");

						uint8_t reg_id = file_reader.read<uint8_t>();
						uint64_t address = file_reader.read<uint64_t>();

						print_arg_reg(reg_id);
						print_arg_address(address);
						break;
					}

					case MOVE_REG_INTO_MEM_64:
					{
						print_instruction("MOVE_REG_INTO_MEM_64");

						uint8_t reg_id = file_reader.read<uint8_t>();
						uint64_t address = file_reader.read<uint64_t>();

						print_arg_reg(reg_id);
						print_arg_address(address);
						break;
					}

					case MOVE_MEM_8_INTO_REG:
					{
						print_instruction("MOVE_MEM_8_INTO_REG");

						uint64_t address = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_address(address);
						print_arg_reg(reg_id);
						break;
					}

					case MOVE_MEM_16_INTO_REG:
					{
						print_instruction("MOVE_MEM_16_INTO_REG");

						uint64_t address = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_address(address);
						print_arg_reg(reg_id);
						break;
					}

					case MOVE_MEM_32_INTO_REG:
					{
						print_instruction("MOVE_MEM_32_INTO_REG");

						uint64_t address = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_address(address);
						print_arg_reg(reg_id);
						break;
					}

					case MOVE_MEM_64_INTO_REG:
					{
						print_instruction("MOVE_MEM_64_INTO_REG");

						uint64_t address = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_address(address);
						print_arg_reg(reg_id);
						break;
					}

					case MOVE_REG_POINTER_8_INTO_REG:
					{
						print_instruction("MOVE_REG_POINTER_8_INTO_REG");

						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						print_arg_reg(reg_id_1);
						print_arg_reg(reg_id_2);
						break;
					}

					case MOVE_REG_POINTER_16_INTO_REG:
					{
						print_instruction("MOVE_REG_POINTER_16_INTO_REG");

						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						print_arg_reg(reg_id_1);
						print_arg_reg(reg_id_2);
						break;
					}

					case MOVE_REG_POINTER_32_INTO_REG:
					{
						print_instruction("MOVE_REG_POINTER_32_INTO_REG");

						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						print_arg_reg(reg_id_1);
						print_arg_reg(reg_id_2);
						break;
					}

					case MOVE_REG_POINTER_64_INTO_REG:
					{
						print_instruction("MOVE_REG_POINTER_64_INTO_REG");
						
						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						print_arg_reg(reg_id_1);
						print_arg_reg(reg_id_2);
						break;
					}

					case MOVE_REG_INTO_REG_POINTER_8:
					{
						print_instruction("MOVE_REG_INTO_REG_POINTER_8");

						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						print_arg_reg(reg_id_1);
						print_arg_reg(reg_id_2);
						break;
					}

					case MOVE_REG_INTO_REG_POINTER_16:
					{
						print_instruction("MOVE_REG_INTO_REG_POINTER_16");

						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						print_arg_reg(reg_id_1);
						print_arg_reg(reg_id_2);
						break;
					}

					case MOVE_REG_INTO_REG_POINTER_32:
					{
						print_instruction("MOVE_REG_INTO_REG_POINTER_32");

						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						print_arg_reg(reg_id_1);
						print_arg_reg(reg_id_2);
						break;
					}

					case MOVE_REG_INTO_REG_POINTER_64:
					{
						print_instruction("MOVE_REG_INTO_REG_POINTER_64");

						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						print_arg_reg(reg_id_1);
						print_arg_reg(reg_id_2);
						break;
					}

					case MOVE_FRAME_OFFSET_8_INTO_REG:
					{
						print_instruction("MOVE_FRAME_OFFSET_8_INTO_REG");

						int64_t offset = file_reader.read<int64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(offset);
						print_arg_reg(reg_id);
						break;
					}

					case MOVE_FRAME_OFFSET_16_INTO_REG:
					{
						print_instruction("MOVE_FRAME_OFFSET_16_INTO_REG");

						int64_t offset = file_reader.read<int64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(offset);
						print_arg_reg(reg_id);
						break;
					}

					case MOVE_FRAME_OFFSET_32_INTO_REG:
					{
						print_instruction("MOVE_FRAME_OFFSET_32_INTO_REG");

						int64_t offset = file_reader.read<int64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(offset);
						print_arg_reg(reg_id);
						break;
					}

					case MOVE_FRAME_OFFSET_64_INTO_REG:
					{
						print_instruction("MOVE_FRAME_OFFSET_64_INTO_REG");

						int64_t offset = file_reader.read<int64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(offset);
						print_arg_reg(reg_id);
						break;
					}

					case MOVE_REG_INTO_FRAME_OFFSET_8:
					{
						print_instruction("MOVE_REG_INTO_FRAME_OFFSET_8");

						uint8_t reg_id = file_reader.read<uint8_t>();
						int64_t offset = file_reader.read<int64_t>();

						print_arg_reg(reg_id);
						print_arg_literal_number(offset);
						break;
					}

					case MOVE_REG_INTO_FRAME_OFFSET_16:
					{
						print_instruction("MOVE_REG_INTO_FRAME_OFFSET_16");

						uint8_t reg_id = file_reader.read<uint8_t>();
						int64_t offset = file_reader.read<int64_t>();

						print_arg_reg(reg_id);
						print_arg_literal_number(offset);
						break;
					}

					case MOVE_REG_INTO_FRAME_OFFSET_32:
					{
						print_instruction("MOVE_REG_INTO_FRAME_OFFSET_32");

						uint8_t reg_id = file_reader.read<uint8_t>();
						int64_t offset = file_reader.read<int64_t>();

						print_arg_reg(reg_id);
						print_arg_literal_number(offset);
						break;
					}

					case MOVE_REG_INTO_FRAME_OFFSET_64:
					{
						print_instruction("MOVE_REG_INTO_FRAME_OFFSET_64");

						uint8_t reg_id = file_reader.read<uint8_t>();
						int64_t offset = file_reader.read<int64_t>();

						print_arg_reg(reg_id);
						print_arg_literal_number(offset);
						break;
					}

					case MOVE_STACK_TOP_OFFSET_8_INTO_REG:
					{
						print_instruction("MOVE_STACK_TOP_OFFSET_8_INTO_REG");

						int64_t offset = file_reader.read<int64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(offset);
						print_arg_reg(reg_id);
						break;
					}

					case MOVE_STACK_TOP_OFFSET_16_INTO_REG:
					{
						print_instruction("MOVE_STACK_TOP_OFFSET_16_INTO_REG");

						int64_t offset = file_reader.read<int64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(offset);
						print_arg_reg(reg_id);
						break;
					}

					case MOVE_STACK_TOP_OFFSET_32_INTO_REG:
					{
						print_instruction("MOVE_STACK_TOP_OFFSET_32_INTO_REG");

						int64_t offset = file_reader.read<int64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(offset);
						print_arg_reg(reg_id);
						break;
					}

					case MOVE_STACK_TOP_OFFSET_64_INTO_REG:
					{
						print_instruction("MOVE_STACK_TOP_OFFSET_64_INTO_REG");

						int64_t offset = file_reader.read<int64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(offset);
						print_arg_reg(reg_id);
						break;
					}

					case MOVE_REG_INTO_STACK_TOP_OFFSET_8:
					{
						print_instruction("MOVE_REG_INTO_STACK_TOP_OFFSET_8");

						uint8_t reg_id = file_reader.read<uint8_t>();
						int64_t offset = file_reader.read<int64_t>();

						print_arg_reg(reg_id);
						print_arg_literal_number(offset);
						break;
					}

					case MOVE_REG_INTO_STACK_TOP_OFFSET_16:
					{
						print_instruction("MOVE_REG_INTO_STACK_TOP_OFFSET_16");

						uint8_t reg_id = file_reader.read<uint8_t>();
						int64_t offset = file_reader.read<int64_t>();

						print_arg_reg(reg_id);
						print_arg_literal_number(offset);
						break;
					}

					case MOVE_REG_INTO_STACK_TOP_OFFSET_32:
					{
						print_instruction("MOVE_REG_INTO_STACK_TOP_OFFSET_32");

						uint8_t reg_id = file_reader.read<uint8_t>();
						int64_t offset = file_reader.read<int64_t>();

						print_arg_reg(reg_id);
						print_arg_literal_number(offset);
						break;
					}

					case MOVE_REG_INTO_STACK_TOP_OFFSET_64:
					{
						print_instruction("MOVE_REG_INTO_STACK_TOP_OFFSET_64");

						uint8_t reg_id = file_reader.read<uint8_t>();
						int64_t offset = file_reader.read<int64_t>();

						print_arg_reg(reg_id);
						print_arg_literal_number(offset);
						break;
					}

					case ADD_8_INTO_REG:
					{
						print_instruction("ADD_8_INTO_REG");

						uint8_t lit = file_reader.read<uint8_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case ADD_16_INTO_REG:
					{
						print_instruction("ADD_16_INTO_REG");

						uint16_t lit = file_reader.read<uint16_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case ADD_32_INTO_REG:
					{
						print_instruction("ADD_32_INTO_REG");

						uint32_t lit = file_reader.read<uint32_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case ADD_64_INTO_REG:
					{
						print_instruction("ADD_64_INTO_REG");

						uint64_t lit = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case ADD_REG_INTO_REG:
					{
						print_instruction("ADD_REG_INTO_REG");

						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						print_arg_reg(reg_id_1);
						print_arg_reg(reg_id_2);
						break;
					}

					case SUBTRACT_8_FROM_REG:
					{
						print_instruction("SUBTRACT_8_FROM_REG");

						uint8_t lit = file_reader.read<uint8_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case SUBTRACT_16_FROM_REG:
					{
						print_instruction("SUBTRACT_16_FROM_REG");

						uint16_t lit = file_reader.read<uint16_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case SUBTRACT_32_FROM_REG:
					{
						print_instruction("SUBTRACT_32_FROM_REG");

						uint32_t lit = file_reader.read<uint32_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case SUBTRACT_64_FROM_REG:
					{
						print_instruction("SUBTRACT_64_FROM_REG");

						uint64_t lit = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case SUBTRACT_REG_FROM_REG:
					{
						print_instruction("SUBTRACT_REG_FROM_REG");

						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						print_arg_reg(reg_id_1);
						print_arg_reg(reg_id_2);
						break;
					}

					case MULTIPLY_8_INTO_REG:
					{
						print_instruction("MULTIPLY_8_INTO_REG");

						uint8_t lit = file_reader.read<uint8_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case MULTIPLY_16_INTO_REG:
					{
						print_instruction("MULTIPLY_16_INTO_REG");

						uint16_t lit = file_reader.read<uint16_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case MULTIPLY_32_INTO_REG:
					{
						print_instruction("MULTIPLY_32_INTO_REG");

						uint32_t lit = file_reader.read<uint32_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case MULTIPLY_64_INTO_REG:
					{
						print_instruction("MULTIPLY_64_INTO_REG");

						uint64_t lit = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case MULTIPLY_REG_INTO_REG:
					{
						print_instruction("MULTIPLY_REG_INTO_REG");

						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						print_arg_reg(reg_id_1);
						print_arg_reg(reg_id_2);
						break;
					}

					case DIVIDE_8_FROM_REG:
					{
						print_instruction("DIVIDE_8_FROM_REG");

						uint8_t lit = file_reader.read<uint8_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case DIVIDE_16_FROM_REG:
					{
						print_instruction("DIVIDE_16_FROM_REG");

						uint16_t lit = file_reader.read<uint16_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case DIVIDE_32_FROM_REG:
					{
						print_instruction("DIVIDE_32_FROM_REG");

						uint32_t lit = file_reader.read<uint32_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case DIVIDE_64_FROM_REG:
					{
						print_instruction("DIVIDE_64_FROM_REG");

						uint64_t lit = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case DIVIDE_REG_FROM_REG:
					{
						print_instruction("DIVIDE_REG_FROM_REG");

						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						print_arg_reg(reg_id_1);
						print_arg_reg(reg_id_2);
						break;
					}

					case TAKE_MODULO_8_OF_REG:
					{
						print_instruction("TAKE_MODULO_8_OF_REG");

						uint8_t lit = file_reader.read<uint8_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case TAKE_MODULO_16_OF_REG:
					{
						print_instruction("TAKE_MODULO_16_OF_REG");

						uint16_t lit = file_reader.read<uint16_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case TAKE_MODULO_32_OF_REG:
					{
						print_instruction("TAKE_MODULO_32_OF_REG");

						uint32_t lit = file_reader.read<uint32_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case TAKE_MODULO_64_OF_REG:
					{
						print_instruction("TAKE_MODULO_64_OF_REG");

						uint64_t lit = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case TAKE_MODULO_REG_OF_REG:
					{
						print_instruction("TAKE_MODULO_8_OF_REG");

						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						print_arg_reg(reg_id_1);
						print_arg_reg(reg_id_2);
						break;
					}

					case AND_8_INTO_REG:
					{
						print_instruction("AND_8_INTO_REG");

						uint8_t lit = file_reader.read<uint8_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case AND_16_INTO_REG:
					{
						print_instruction("AND_16_INTO_REG");

						uint16_t lit = file_reader.read<uint16_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case AND_32_INTO_REG:
					{
						print_instruction("AND_32_INTO_REG");

						uint32_t lit = file_reader.read<uint32_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case AND_64_INTO_REG:
					{
						print_instruction("AND_64_INTO_REG");

						uint64_t lit = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case AND_REG_INTO_REG:
					{
						print_instruction("AND_REG_INTO_REG");

						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						print_arg_reg(reg_id_1);
						print_arg_reg(reg_id_2);
						break;
					}

					case OR_8_INTO_REG:
					{
						print_instruction("OR_8_INTO_REG");

						uint8_t lit = file_reader.read<uint8_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case OR_16_INTO_REG:
					{
						print_instruction("OR_16_INTO_REG");
						
						uint16_t lit = file_reader.read<uint16_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case OR_32_INTO_REG:
					{
						print_instruction("OR832_INTO_REG");

						uint32_t lit = file_reader.read<uint32_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case OR_64_INTO_REG:
					{
						print_instruction("OR_64_INTO_REG");

						uint64_t lit = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case OR_REG_INTO_REG:
					{
						print_instruction("OR_REG_INTO_REG");

						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						print_arg_reg(reg_id_1);
						print_arg_reg(reg_id_2);
						break;
					}

					case XOR_8_INTO_REG:
					{
						print_instruction("XOR_8_INTO_REG");

						uint8_t lit = file_reader.read<uint8_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case XOR_16_INTO_REG:
					{
						print_instruction("XOR_16_INTO_REG");

						uint16_t lit = file_reader.read<uint16_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case XOR_32_INTO_REG:
					{
						print_instruction("XOR_32_INTO_REG");

						uint32_t lit = file_reader.read<uint32_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case XOR_64_INTO_REG:
					{
						print_instruction("XOR_64_INTO_REG");

						uint64_t lit = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case XOR_REG_INTO_REG:
					{
						print_instruction("XOR_REG_INTO_REG");
						
						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						print_arg_reg(reg_id_1);
						print_arg_reg(reg_id_2);
						break;
					}

					case LEFT_SHIFT_REG_BY_8:
					{
						print_instruction("LEFT_SHIFT_REG_BY_8");

						uint8_t lit = file_reader.read<uint8_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}


					case LEFT_SHIFT_REG_BY_REG:
					{
						print_instruction("LEFT_SHIFT_REG_BY_REG");

						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						print_arg_reg(reg_id_1);
						print_arg_reg(reg_id_2);
						break;
					}

					case RIGHT_SHIFT_REG_BY_8:
					{
						print_instruction("RIGHT_SHIFT_REG_BY_8");

						uint8_t lit = file_reader.read<uint8_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case RIGHT_SHIFT_REG_BY_REG:
					{
						print_instruction("RIGHT_SHIFT_REG_BY_REG");

						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						print_arg_reg(reg_id_1);
						print_arg_reg(reg_id_2);
						break;
					}

					case INCREMENT_REG:
					{
						print_instruction("INCREMENT_REG");

						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_reg(reg_id);
						break;
					}

					case DECREMENT_REG:
					{
						print_instruction("DECREMENT_REG");

						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_reg(reg_id);
						break;
					}

					case NOT_REG:
					{
						print_instruction("NOT_REG");

						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_reg(reg_id);
						break;
					}

					case COMPARE_8_TO_REG:
					{
						print_instruction("COMPARE_8_TO_REG");

						uint8_t lit = file_reader.read<uint8_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case COMPARE_16_TO_REG:
					{
						print_instruction("COMPARE_16_TO_REG");

						uint16_t lit = file_reader.read<uint16_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case COMPARE_32_TO_REG:
					{
						print_instruction("COMPARE_32_TO_REG");

						uint32_t lit = file_reader.read<uint32_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case COMPARE_64_TO_REG:
					{
						print_instruction("COMPARE_64_TO_REG");

						uint64_t lit = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						print_arg_reg(reg_id);
						break;
					}

					case COMPARE_REG_TO_8:
					{
						print_instruction("COMPARE_REG_TO_8");

						uint8_t reg_id = file_reader.read<uint8_t>();
						uint8_t lit = file_reader.read<uint8_t>();

						print_arg_reg(reg_id);
						print_arg_literal_number(lit);
						break;
					}

					case COMPARE_REG_TO_16:
					{
						print_instruction("COMPARE_REG_TO_16");

						uint8_t reg_id = file_reader.read<uint8_t>();
						uint16_t lit = file_reader.read<uint16_t>();

						print_arg_reg(reg_id);
						print_arg_literal_number(lit);
						break;
					}

					case COMPARE_REG_TO_32:
					{
						print_instruction("COMPARE_REG_TO_32");

						uint8_t reg_id = file_reader.read<uint8_t>();
						uint32_t lit = file_reader.read<uint32_t>();

						print_arg_reg(reg_id);
						print_arg_literal_number(lit);
						break;
					}

					case COMPARE_REG_TO_64:
					{
						print_instruction("COMPARE_REG_TO_64");

						uint8_t reg_id = file_reader.read<uint8_t>();
						uint64_t lit = file_reader.read<uint64_t>();

						print_arg_reg(reg_id);
						print_arg_literal_number(lit);
						break;
					}

					case COMPARE_REG_TO_REG:
					{
						print_instruction("COMPARE_REG_TO_REG");

						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						print_arg_reg(reg_id_1);
						print_arg_reg(reg_id_2);
						break;
					}

					case SET_REG_IF_GREATER:
					{
						print_instruction("SET_REG_IF_GREATER");

						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_reg(reg_id);
						break;
					}

					case SET_REG_IF_GREATER_OR_EQUAL:
					{
						print_instruction("SET_REG_IF_GREATER_OR_EQUAL");

						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_reg(reg_id);
						break;
					}

					case SET_REG_IF_LESS:
					{
						print_instruction("SET_REG_IF_LESS");

						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_reg(reg_id);
						break;
					}

					case SET_REG_IF_LESS_OR_EQUAL:
					{
						print_instruction("SET_REG_IF_LESS_OR_EQUAL");

						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_reg(reg_id);
						break;
					}

					case SET_REG_IF_EQUAL:
					{
						print_instruction("SET_REG_IF_EQUAL");

						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_reg(reg_id);
						break;
					}

					case SET_REG_IF_NOT_EQUAL:
					{
						print_instruction("SET_REG_IF_NOT_EQUAL");

						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_reg(reg_id);
						break;
					}

					case JUMP:
					{
						print_instruction("JUMP");

						uint64_t address = file_reader.read<uint64_t>();

						print_arg_address(address);
						break;
					}

					case JUMP_IF_GREATER:
					{
						print_instruction("JUMP_IF_GREATER");

						uint64_t address = file_reader.read<uint64_t>();

						print_arg_address(address);
						break;
					}

					case JUMP_IF_GREATER_OR_EQUAL:
					{
						print_instruction("JUMP_IF_GREATER_OR_EQUAL");

						uint64_t address = file_reader.read<uint64_t>();

						print_arg_address(address);
						break;
					}

					case JUMP_IF_LESS:
					{
						print_instruction("JUMP_IF_LESS");

						uint64_t address = file_reader.read<uint64_t>();

						print_arg_address(address);
						break;
					}

					case JUMP_IF_LESS_OR_EQUAL:
					{
						print_instruction("JUMP_IF_LESS_OR_EQUAL");

						uint64_t address = file_reader.read<uint64_t>();

						print_arg_address(address);
						break;
					}

					case JUMP_IF_EQUAL:
					{
						print_instruction("JUMP_IF_EQUAL");

						uint64_t address = file_reader.read<uint64_t>();

						print_arg_address(address);
						break;
					}

					case JUMP_IF_NOT_EQUAL:
					{
						print_instruction("JUMP_IF_NOT_EQUAL");

						uint64_t address = file_reader.read<uint64_t>();

						print_arg_address(address);
						break;
					}

					case PUSH_8:
					{
						print_instruction("PUSH_8");

						uint8_t lit = file_reader.read<uint8_t>();

						print_arg_literal_number(lit);
						break;
					}

					case PUSH_16:
					{
						print_instruction("PUSH_16");

						uint16_t lit = file_reader.read<uint16_t>();

						print_arg_literal_number(lit);
						break;
					}

					case PUSH_32:
					{
						print_instruction("PUSH_32");

						uint32_t lit = file_reader.read<uint32_t>();

						print_arg_literal_number(lit);
						break;
					}

					case PUSH_64:
					{
						print_instruction("PUSH_64");

						uint64_t lit = file_reader.read<uint64_t>();

						print_arg_literal_number(lit);
						break;
					}

					case PUSH_REG_8:
					{
						print_instruction("PUSH_REG_8");

						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_reg(reg_id);
						break;
					}

					case PUSH_REG_16:
					{
						print_instruction("PUSH_REG_16");

						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_reg(reg_id);
						break;
					}

					case PUSH_REG_32:
					{
						print_instruction("PUSH_REG_32");

						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_reg(reg_id);
						break;
					}

					case PUSH_REG_64:
					{
						print_instruction("PUSH_REG_64");

						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_reg(reg_id);
						break;
					}

					case POP_8_INTO_REG:
					{
						print_instruction("POP_8_INTO_REG");

						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_reg(reg_id);
						break;
					}

					case POP_16_INTO_REG:
					{
						print_instruction("POP_16_INTO_REG");

						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_reg(reg_id);
						break;
					}

					case POP_32_INTO_REG:
					{
						print_instruction("POP_32_INTO_REG");

						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_reg(reg_id);
						break;
					}

					case POP_64_INTO_REG:
					{
						print_instruction("POP_64_INTO_REG");

						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_reg(reg_id);
						break;
					}

					case CALL:
					{
						print_instruction("CALL");

						uint64_t address = file_reader.read<uint64_t>();

						print_arg_address(address);
						break;
					}

					case RETURN:
					{
						print_instruction("RETURN");
						break;
					}

					case ALLOCATE_STACK:
					{
						print_instruction("ALLOCATE_STACK");

						uint64_t size = file_reader.read<uint64_t>();

						print_arg_literal_number(size);
						break;
					}

					case DEALLOCATE_STACK:
					{
						print_instruction("ALLOCATE_STACK");

						uint64_t size = file_reader.read<uint64_t>();

						print_arg_literal_number(size);
						break;
					}

					case LOG_REG:
					{
						print_instruction("LOG_REG");

						uint8_t reg_id = file_reader.read<uint8_t>();

						print_arg_reg(reg_id);
						break;
					}
				}

				end_args();
			}
		}
};

#endif