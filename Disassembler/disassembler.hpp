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

		Disassembler(FILE *file_in) : file_reader(file_in) {}

		void disassemble(FILE *file_out)
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

			while ((instruction = (Instruction) file_reader.read<uint16_t>()) != (1 << 16) - 1) {
				fprintf(file_out, "0x%04lx    ",
					PROGRAM_START + file_reader.read_bytes - 18);

				switch (instruction) {
					default:
					{
						fprintf(file_out, "dafuq is instruction %hu?\n", instruction);
						abort();
					}

					case MOVE_8_INTO_REG:
					{
						uint8_t lit = file_reader.read<uint8_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "MOVE_8_INTO_REG %hhu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case MOVE_16_INTO_REG:
					{
						uint16_t lit = file_reader.read<uint16_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "MOVE_16_INTO_REG %hu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case MOVE_32_INTO_REG:
					{
						uint32_t lit = file_reader.read<uint32_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "MOVE_32_INTO_REG %u, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case MOVE_64_INTO_REG:
					{
						uint64_t lit = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "MOVE_8_INTO_REG %lu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case MOVE_8_INTO_MEM:
					{
						uint8_t lit = file_reader.read<uint8_t>();
						uint64_t address = file_reader.read<uint64_t>();

						fprintf(file_out, "MOVE_8_INTO_MEM %hhu, 0x%lx\n",
							lit, address);
						break;
					}

					case MOVE_16_INTO_MEM:
					{
						uint16_t lit = file_reader.read<uint16_t>();
						uint64_t address = file_reader.read<uint64_t>();

						fprintf(file_out, "MOVE_16_INTO_MEM %hu, 0x%lx\n",
							lit, address);
						break;
					}

					case MOVE_32_INTO_MEM:
					{
						uint32_t lit = file_reader.read<uint32_t>();
						uint64_t address = file_reader.read<uint64_t>();

						fprintf(file_out, "MOVE_32_INTO_MEM %u, 0x%lx\n",
							lit, address);
						break;
					}

					case MOVE_64_INTO_MEM:
					{
						uint64_t lit = file_reader.read<uint64_t>();
						uint64_t address = file_reader.read<uint64_t>();

						fprintf(file_out, "MOVE_64_INTO_MEM %lu, 0x%lx\n",
							lit, address);
						break;
					}

					case MOVE_REG_INTO_REG:
					{
						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						fprintf(file_out, "MOVE_REG_INTO_REG %s, %s\n",
							CPU::reg_to_str(reg_id_1), CPU::reg_to_str(reg_id_2));
						break;
					}

					case MOVE_REG_INTO_MEM:
					{
						uint8_t reg_id = file_reader.read<uint8_t>();
						uint64_t address = file_reader.read<uint64_t>();

						fprintf(file_out, "MOVE_REG_INTO_MEM %s, 0x%lx\n",
							CPU::reg_to_str(reg_id), address);
						break;
					}

					case MOVE_MEM_8_INTO_REG:
					{
						uint64_t address = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "MOVE_MEM_8_INTO_REG 0x%lx, %s\n",
							address, CPU::reg_to_str(reg_id));
						break;
					}

					case MOVE_MEM_16_INTO_REG:
					{
						uint64_t address = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "MOVE_MEM_16_INTO_REG 0x%lx, %s\n",
							address, CPU::reg_to_str(reg_id));
						break;
					}

					case MOVE_MEM_32_INTO_REG:
					{
						uint64_t address = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "MOVE_MEM_32_INTO_REG 0x%lx, %s\n",
							address, CPU::reg_to_str(reg_id));
						break;
					}

					case MOVE_MEM_64_INTO_REG:
					{
						uint64_t address = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "MOVE_MEM_64_INTO_REG 0x%lx, %s\n",
							address, CPU::reg_to_str(reg_id));
						break;
					}

					case MOVE_REG_POINTER_8_INTO_REG:
					{
						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						fprintf(file_out, "MOVE_REG_POINTER_8_INTO_REG %s, %s\n",
							CPU::reg_to_str(reg_id_1), CPU::reg_to_str(reg_id_2));
						break;
					}

					case MOVE_REG_POINTER_16_INTO_REG:
					{
						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						fprintf(file_out, "MOVE_REG_POINTER_16_INTO_REG %s, %s\n",
							CPU::reg_to_str(reg_id_1), CPU::reg_to_str(reg_id_2));
						break;
					}

					case MOVE_REG_POINTER_32_INTO_REG:
					{
						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						fprintf(file_out, "MOVE_REG_POINTER_32_INTO_REG %s, %s\n",
							CPU::reg_to_str(reg_id_1), CPU::reg_to_str(reg_id_2));
						break;
					}

					case MOVE_REG_POINTER_64_INTO_REG:
					{
						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						fprintf(file_out, "MOVE_REG_POINTER_64_INTO_REG %s, %s\n",
							CPU::reg_to_str(reg_id_1), CPU::reg_to_str(reg_id_2));
						break;
					}

					case MOVE_REG_INTO_REG_POINTER_8:
					{
						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						fprintf(file_out, "MOVE_REG_INTO_REG_POINTER_8 %s, %s\n",
							CPU::reg_to_str(reg_id_1), CPU::reg_to_str(reg_id_2));
						break;
					}

					case MOVE_REG_INTO_REG_POINTER_16:
					{
						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						fprintf(file_out, "MOVE_REG_INTO_REG_POINTER_16 %s, %s\n",
							CPU::reg_to_str(reg_id_1), CPU::reg_to_str(reg_id_2));
						break;
					}

					case MOVE_REG_INTO_REG_POINTER_32:
					{
						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						fprintf(file_out, "MOVE_REG_INTO_REG_POINTER_32 %s, %s\n",
							CPU::reg_to_str(reg_id_1), CPU::reg_to_str(reg_id_2));
						break;
					}

					case MOVE_REG_INTO_REG_POINTER_64:
					{
						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						fprintf(file_out, "MOVE_REG_INTO_REG_POINTER_64 %s, %s\n",
							CPU::reg_to_str(reg_id_1), CPU::reg_to_str(reg_id_2));
						break;
					}

					case MOVE_FRAME_OFFSET_8_INTO_REG:
					{
						int64_t offset = file_reader.read<int64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "MOVE_FRAME_OFFSET_8_INTO_REG %ld, %s\n",
							offset, CPU::reg_to_str(reg_id));
						break;
					}

					case MOVE_FRAME_OFFSET_16_INTO_REG:
					{
						int64_t offset = file_reader.read<int64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "MOVE_FRAME_OFFSET_16_INTO_REG %ld, %s\n",
							offset, CPU::reg_to_str(reg_id));
						break;
					}

					case MOVE_FRAME_OFFSET_32_INTO_REG:
					{
						int64_t offset = file_reader.read<int64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "MOVE_FRAME_OFFSET_32_INTO_REG %ld, %s\n",
							offset, CPU::reg_to_str(reg_id));
						break;
					}

					case MOVE_FRAME_OFFSET_64_INTO_REG:
					{
						int64_t offset = file_reader.read<int64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "MOVE_FRAME_OFFSET_64_INTO_REG %ld, %s\n",
							offset, CPU::reg_to_str(reg_id));
						break;
					}

					case MOVE_REG_INTO_FRAME_OFFSET_8:
					{
						uint8_t reg_id = file_reader.read<uint8_t>();
						int64_t offset = file_reader.read<int64_t>();

						fprintf(file_out, "MOVE_REG_INTO_FRAME_OFFSET_8 %s, %ld\n",
							CPU::reg_to_str(reg_id), offset);
						break;
					}

					case MOVE_REG_INTO_FRAME_OFFSET_16:
					{
						uint8_t reg_id = file_reader.read<uint8_t>();
						int64_t offset = file_reader.read<int64_t>();

						fprintf(file_out, "MOVE_REG_INTO_FRAME_OFFSET_16 %s, %ld\n",
							CPU::reg_to_str(reg_id), offset);
						break;
					}

					case MOVE_REG_INTO_FRAME_OFFSET_32:
					{
						uint8_t reg_id = file_reader.read<uint8_t>();
						int64_t offset = file_reader.read<int64_t>();

						fprintf(file_out, "MOVE_REG_INTO_FRAME_OFFSET_32 %s, %ld\n",
							CPU::reg_to_str(reg_id), offset);
						break;
					}

					case MOVE_REG_INTO_FRAME_OFFSET_64:
					{
						uint8_t reg_id = file_reader.read<uint8_t>();
						int64_t offset = file_reader.read<int64_t>();

						fprintf(file_out, "MOVE_REG_INTO_FRAME_OFFSET_64 %s, %ld\n",
							CPU::reg_to_str(reg_id), offset);
						break;
					}

					case MOVE_STACK_TOP_OFFSET_8_INTO_REG:
					{
						int64_t offset = file_reader.read<int64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "MOVE_STACK_TOP_OFFSET_8_INTO_REG %ld, %s\n",
							offset, CPU::reg_to_str(reg_id));
						break;
					}

					case MOVE_STACK_TOP_OFFSET_16_INTO_REG:
					{
						int64_t offset = file_reader.read<int64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "MOVE_STACK_TOP_OFFSET_16_INTO_REG %ld, %s\n",
							offset, CPU::reg_to_str(reg_id));
						break;
					}

					case MOVE_STACK_TOP_OFFSET_32_INTO_REG:
					{
						int64_t offset = file_reader.read<int64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "MOVE_STACK_TOP_OFFSET_32_INTO_REG %ld, %s\n",
							offset, CPU::reg_to_str(reg_id));
						break;
					}

					case MOVE_STACK_TOP_OFFSET_64_INTO_REG:
					{
						int64_t offset = file_reader.read<int64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "MOVE_STACK_TOP_OFFSET_64_INTO_REG %ld, %s\n",
							offset, CPU::reg_to_str(reg_id));
						break;
					}

					case MOVE_REG_INTO_STACK_TOP_OFFSET_8:
					{
						uint8_t reg_id = file_reader.read<uint8_t>();
						int64_t offset = file_reader.read<int64_t>();

						fprintf(file_out, "MOVE_REG_INTO_STACK_TOP_OFFSET_8 %s, %ld\n",
							CPU::reg_to_str(reg_id), offset);
						break;
					}

					case MOVE_REG_INTO_STACK_TOP_OFFSET_16:
					{
						uint8_t reg_id = file_reader.read<uint8_t>();
						int64_t offset = file_reader.read<int64_t>();

						fprintf(file_out, "MOVE_REG_INTO_STACK_TOP_OFFSET_16 %s, %ld\n",
							CPU::reg_to_str(reg_id), offset);
						break;
					}

					case MOVE_REG_INTO_STACK_TOP_OFFSET_32:
					{
						uint8_t reg_id = file_reader.read<uint8_t>();
						int64_t offset = file_reader.read<int64_t>();

						fprintf(file_out, "MOVE_REG_INTO_STACK_TOP_OFFSET_32 %s, %ld\n",
							CPU::reg_to_str(reg_id), offset);
						break;
					}

					case MOVE_REG_INTO_STACK_TOP_OFFSET_64:
					{
						uint8_t reg_id = file_reader.read<uint8_t>();
						int64_t offset = file_reader.read<int64_t>();

						fprintf(file_out, "MOVE_REG_INTO_STACK_TOP_OFFSET_64 %s, %ld\n",
							CPU::reg_to_str(reg_id), offset);
						break;
					}

					case ADD_8_INTO_REG:
					{
						uint8_t lit = file_reader.read<uint8_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "ADD_8_INTO_REG %hhu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case ADD_16_INTO_REG:
					{
						uint16_t lit = file_reader.read<uint16_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "ADD_16_INTO_REG %hu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case ADD_32_INTO_REG:
					{
						uint32_t lit = file_reader.read<uint32_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "ADD_32_INTO_REG %u, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case ADD_64_INTO_REG:
					{
						uint64_t lit = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "ADD_64_INTO_REG %lu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case ADD_REG_INTO_REG:
					{
						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						fprintf(file_out, "ADD_REG_INTO_REG %s, %s\n",
							CPU::reg_to_str(reg_id_1), CPU::reg_to_str(reg_id_2));
						break;
					}

					case SUBTRACT_8_FROM_REG:
					{
						uint8_t lit = file_reader.read<uint8_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "SUBTRACT_8_FROM_REG %hhu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case SUBTRACT_16_FROM_REG:
					{
						uint16_t lit = file_reader.read<uint16_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "SUBTRACT_16_FROM_REG %hu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case SUBTRACT_32_FROM_REG:
					{
						uint32_t lit = file_reader.read<uint32_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "SUBTRACT_32_FROM_REG %u, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case SUBTRACT_64_FROM_REG:
					{
						uint64_t lit = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "SUBTRACT_64_FROM_REG %lu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case SUBTRACT_REG_FROM_REG:
					{
						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						fprintf(file_out, "SUBTRACT_REG_FROM_REG %s, %s\n",
							CPU::reg_to_str(reg_id_1), CPU::reg_to_str(reg_id_2));
						break;
					}

					case MULTIPLY_8_INTO_REG:
					{
						uint8_t lit = file_reader.read<uint8_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "MULTIPLY_8_INTO_REG %hhu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case MULTIPLY_16_INTO_REG:
					{
						uint16_t lit = file_reader.read<uint16_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "MULTIPLY_16_INTO_REG %hu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case MULTIPLY_32_INTO_REG:
					{
						uint32_t lit = file_reader.read<uint32_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "MULTIPLY_32_INTO_REG %u, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case MULTIPLY_64_INTO_REG:
					{
						uint64_t lit = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "MULTIPLY_64_INTO_REG %lu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case MULTIPLY_REG_INTO_REG:
					{
						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						fprintf(file_out, "MULTIPLY_REG_INTO_REG %s, %s\n",
							CPU::reg_to_str(reg_id_1), CPU::reg_to_str(reg_id_2));
						break;
					}

					case DIVIDE_8_FROM_REG:
					{
						uint8_t lit = file_reader.read<uint8_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "DIVIDE_8_FROM_REG %hhu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case DIVIDE_16_FROM_REG:
					{
						uint16_t lit = file_reader.read<uint16_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "DIVIDE_16_FROM_REG %hu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case DIVIDE_32_FROM_REG:
					{
						uint32_t lit = file_reader.read<uint32_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "DIVIDE_32_FROM_REG %u, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case DIVIDE_64_FROM_REG:
					{
						uint64_t lit = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "DIVIDE_64_FROM_REG %lu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case DIVIDE_REG_FROM_REG:
					{
						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						fprintf(file_out, "DIVIDE_REG_FROM_REG %s, %s\n",
							CPU::reg_to_str(reg_id_1), CPU::reg_to_str(reg_id_2));
						break;
					}

					case TAKE_MODULO_8_OF_REG:
					{
						uint8_t lit = file_reader.read<uint8_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "TAKE_MODULO_8_OF_REG %hhu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case TAKE_MODULO_16_OF_REG:
					{
						uint16_t lit = file_reader.read<uint16_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "TAKE_MODULO_16_OF_REG %hu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case TAKE_MODULO_32_OF_REG:
					{
						uint32_t lit = file_reader.read<uint32_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "TAKE_MODULO_32_OF_REG %u, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case TAKE_MODULO_64_OF_REG:
					{
						uint64_t lit = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "TAKE_MODULO_64_OF_REG %lu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case TAKE_MODULO_REG_OF_REG:
					{
						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						fprintf(file_out, "TAKE_MODULO_REG_OF_REG %s, %s\n",
							CPU::reg_to_str(reg_id_1), CPU::reg_to_str(reg_id_2));
						break;
					}

					case AND_8_INTO_REG:
					{
						uint8_t lit = file_reader.read<uint8_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "AND_8_INTO_REG %hhu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case AND_16_INTO_REG:
					{
						uint16_t lit = file_reader.read<uint16_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "AND_16_INTO_REG %hu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case AND_32_INTO_REG:
					{
						uint32_t lit = file_reader.read<uint32_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "AND_32_INTO_REG %u, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case AND_64_INTO_REG:
					{
						uint64_t lit = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "AND_64_INTO_REG %lu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case AND_REG_INTO_REG:
					{
						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						fprintf(file_out, "AND_REG_INTO_REG %s, %s\n",
							CPU::reg_to_str(reg_id_1), CPU::reg_to_str(reg_id_2));
						break;
					}

					case OR_8_INTO_REG:
					{
						uint8_t lit = file_reader.read<uint8_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "OR_8_INTO_REG %hhu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case OR_16_INTO_REG:
					{
						uint16_t lit = file_reader.read<uint16_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "OR_16_INTO_REG %hu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case OR_32_INTO_REG:
					{
						uint32_t lit = file_reader.read<uint32_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "OR_32_INTO_REG %u, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case OR_64_INTO_REG:
					{
						uint64_t lit = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "OR_64_INTO_REG %lu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case OR_REG_INTO_REG:
					{
						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						fprintf(file_out, "OR_REG_INTO_REG %s, %s\n",
							CPU::reg_to_str(reg_id_1), CPU::reg_to_str(reg_id_2));
						break;
					}

					case XOR_8_INTO_REG:
					{
						uint8_t lit = file_reader.read<uint8_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "XOR_8_INTO_REG %hhu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case XOR_16_INTO_REG:
					{
						uint16_t lit = file_reader.read<uint16_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "XOR_16_INTO_REG %hu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case XOR_32_INTO_REG:
					{
						uint32_t lit = file_reader.read<uint32_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "XOR_32_INTO_REG %u, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case XOR_64_INTO_REG:
					{
						uint64_t lit = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "XOR_64_INTO_REG %lu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case XOR_REG_INTO_REG:
					{
						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						fprintf(file_out, "XOR_REG_INTO_REG %s, %s\n",
							CPU::reg_to_str(reg_id_1), CPU::reg_to_str(reg_id_2));
						break;
					}

					case LEFT_SHIFT_REG_BY_8:
					{
						uint8_t lit = file_reader.read<uint8_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "LEFT_SHIFT_REG_BY_8 %hhu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}


					case LEFT_SHIFT_REG_BY_REG:
					{
						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						fprintf(file_out, "LEFT_SHIFT_REG_BY_REG %s, %s\n",
							CPU::reg_to_str(reg_id_1), CPU::reg_to_str(reg_id_2));
						break;
					}

					case RIGHT_SHIFT_REG_BY_8:
					{
						uint8_t lit = file_reader.read<uint8_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "RIGHT_SHIFT_REG_BY_8 %hhu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case RIGHT_SHIFT_REG_BY_REG:
					{
						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						fprintf(file_out, "DIVIDE_REG_FROM_REG %s, %s\n",
							CPU::reg_to_str(reg_id_1), CPU::reg_to_str(reg_id_2));
						break;
					}

					case INCREMENT_REG:
					{
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "INCREMENT_REG %s\n",
							CPU::reg_to_str(reg_id));
						break;
					}

					case DECREMENT_REG:
					{
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "DECREMENT_REG %s\n",
							CPU::reg_to_str(reg_id));
						break;
					}

					case NOT_REG:
					{
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "NOT_REG %s\n",
							CPU::reg_to_str(reg_id));
						break;
					}

					case COMPARE_8_TO_REG:
					{
						uint8_t lit = file_reader.read<uint8_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "COMPARE_8_TO_REG %hhu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case COMPARE_16_TO_REG:
					{
						uint16_t lit = file_reader.read<uint16_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "COMPARE_16_TO_REG %hu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case COMPARE_32_TO_REG:
					{
						uint32_t lit = file_reader.read<uint32_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "COMPARE_32_TO_REG %u, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case COMPARE_64_TO_REG:
					{
						uint64_t lit = file_reader.read<uint64_t>();
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "COMPARE_64_TO_REG %lu, %s\n",
							lit, CPU::reg_to_str(reg_id));
						break;
					}

					case COMPARE_REG_TO_8:
					{
						uint8_t reg_id = file_reader.read<uint8_t>();
						uint8_t lit = file_reader.read<uint8_t>();

						fprintf(file_out, "COMPARE_REG_TO_8 %s, %hhu\n",
							CPU::reg_to_str(reg_id), lit);
						break;
					}

					case COMPARE_REG_TO_16:
					{
						uint8_t reg_id = file_reader.read<uint8_t>();
						uint16_t lit = file_reader.read<uint16_t>();

						fprintf(file_out, "COMPARE_REG_TO_16 %s, %hu\n",
							CPU::reg_to_str(reg_id), lit);
						break;
					}

					case COMPARE_REG_TO_32:
					{
						uint8_t reg_id = file_reader.read<uint8_t>();
						uint32_t lit = file_reader.read<uint32_t>();

						fprintf(file_out, "COMPARE_REG_TO_32 %s, %u\n",
							CPU::reg_to_str(reg_id), lit);
						break;
					}

					case COMPARE_REG_TO_64:
					{
						uint8_t reg_id = file_reader.read<uint8_t>();
						uint64_t lit = file_reader.read<uint64_t>();

						fprintf(file_out, "COMPARE_REG_TO_64 %s, %lu\n",
							CPU::reg_to_str(reg_id), lit);
						break;
					}

					case COMPARE_REG_TO_REG:
					{
						uint8_t reg_id_1 = file_reader.read<uint8_t>();
						uint8_t reg_id_2 = file_reader.read<uint8_t>();

						fprintf(file_out, "COMPARE_REG_TO_8 %s, %s\n",
							CPU::reg_to_str(reg_id_1), CPU::reg_to_str(reg_id_2));
						break;
					}

					case JUMP:
					{
						uint64_t address = file_reader.read<uint64_t>();

						fprintf(file_out, "JUMP 0x%lx\n", address);
						break;
					}

					case JUMP_IF_GREATER:
					{
						uint64_t address = file_reader.read<uint64_t>();

						fprintf(file_out, "JUMP_IF_GREATER 0x%lx\n", address);
						break;
					}

					case JUMP_IF_GREATER_OR_EQUAL:
					{
						uint64_t address = file_reader.read<uint64_t>();

						fprintf(file_out, "JUMP_IF_GREATER_OR_EQUAL 0x%lx\n", address);
						break;
					}

					case JUMP_IF_LESS:
					{
						uint64_t address = file_reader.read<uint64_t>();

						fprintf(file_out, "JUMP_IF_LESS 0x%lx\n", address);
						break;
					}

					case JUMP_IF_LESS_OR_EQUAL:
					{
						uint64_t address = file_reader.read<uint64_t>();

						fprintf(file_out, "JUMP_IF_LESS_OR_EQUAL 0x%lx\n", address);
						break;
					}

					case JUMP_IF_EQUAL:
					{
						uint64_t address = file_reader.read<uint64_t>();

						fprintf(file_out, "JUMP_IF_EQUAL 0x%lx\n", address);
						break;
					}

					case PUSH_8:
					{
						uint8_t lit = file_reader.read<uint8_t>();

						fprintf(file_out, "PUSH_8 %hhu\n",
							lit);
						break;
					}

					case PUSH_16:
					{
						uint16_t lit = file_reader.read<uint16_t>();

						fprintf(file_out, "PUSH_16 %hu\n",
							lit);
						break;
					}

					case PUSH_32:
					{
						uint32_t lit = file_reader.read<uint32_t>();

						fprintf(file_out, "PUSH_32 %u\n",
							lit);
						break;
					}

					case PUSH_64:
					{
						uint64_t lit = file_reader.read<uint64_t>();

						fprintf(file_out, "PUSH_64 %lu\n",
							lit);
						break;
					}

					case PUSH_REG_8:
					{
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "PUSH_REG_8 %s\n",
							CPU::reg_to_str(reg_id));
						break;
					}

					case PUSH_REG_16:
					{
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "PUSH_REG_16 %s\n",
							CPU::reg_to_str(reg_id));
						break;
					}

					case PUSH_REG_32:
					{
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "PUSH_REG_32 %s\n",
							CPU::reg_to_str(reg_id));
						break;
					}

					case PUSH_REG_64:
					{
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "PUSH_REG_64 %s\n",
							CPU::reg_to_str(reg_id));
						break;
					}

					case POP_8_INTO_REG:
					{
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "POP_8_INTO_REG %s\n",
							CPU::reg_to_str(reg_id));
						break;
					}

					case POP_16_INTO_REG:
					{
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "POP_16_INTO_REG %s\n",
							CPU::reg_to_str(reg_id));
						break;
					}

					case POP_32_INTO_REG:
					{
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "POP_32_INTO_REG %s\n",
							CPU::reg_to_str(reg_id));
						break;
					}

					case POP_64_INTO_REG:
					{
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "POP_64_INTO_REG %s\n",
							CPU::reg_to_str(reg_id));
						break;
					}

					case CALL:
					{
						uint64_t address = file_reader.read<uint64_t>();

						fprintf(file_out, "CALL 0x%lx\n",
							address);
						break;
					}

					case RETURN:
					{
						fprintf(file_out, "RETURN\n");
						break;
					}

					case ALLOCATE_STACK:
					{
						uint64_t size = file_reader.read<uint64_t>();
						fprintf(file_out, "ALLOCATE_STACK %lu\n", size);
						break;
					}

					case DEALLOCATE_STACK:
					{
						uint64_t size = file_reader.read<uint64_t>();
						fprintf(file_out, "DEALLOCATE_STACK %lu\n", size);
						break;
					}

					case LOG_REG:
					{
						uint8_t reg_id = file_reader.read<uint8_t>();

						fprintf(file_out, "LOG_REG %s\n", CPU::reg_to_str(reg_id));
						break;
					}
				}
			}
		}
};

#endif