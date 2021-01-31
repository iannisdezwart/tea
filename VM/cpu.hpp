#ifndef TEA_CPU_HEADER
#define TEA_CPU_HEADER

#include <bits/stdc++.h>

#include "memory.hpp"
#include "../Compiler/byte_code.hpp"

using namespace std;

class CPU {
	public:
		Memory *memory;
		size_t program_size;
		size_t stack_size;

		uint64_t r_0 = 0;
		#define R_0_ID 0
		uint64_t r_1 = 0;
		#define R_1_ID 1
		uint64_t r_2 = 0;
		#define R_2_ID 2
		uint64_t r_3 = 0;
		#define R_3_ID 3

		uint8_t *r_instruction_p;
		#define R_INSTRUCTION_P_ID 4
		uint8_t *r_stack_p;
		#define R_STACK_P_ID 5
		uint8_t *r_frame_p;
		#define R_FRAME_P_ID 6

		uint64_t r_accumulator = 0;
		#define R_ACCUMULATOR_ID 7

		CPU(MemoryBuilder& program, size_t stack_size)
		{
			this->stack_size = stack_size;
			program_size = program.i;
			memory = program.build(stack_size);

			r_instruction_p = memory->location();
			r_stack_p = r_instruction_p + program_size;
			r_frame_p = r_stack_p;
		}

		uint64_t get_reg_by_id(uint8_t id)
		{
			switch (id) {
				case R_0_ID:
					return r_0;
				case R_1_ID:
					return r_1;
				case R_2_ID:
					return r_2;
				case R_3_ID:
					return r_3;
				case R_INSTRUCTION_P_ID:
					return (uint64_t) r_instruction_p;
				case R_STACK_P_ID:
					return (uint64_t) r_stack_p;
				case R_FRAME_P_ID:
					return (uint64_t) r_frame_p;
				case R_ACCUMULATOR_ID:
					return r_accumulator;
				default:
					printf("Unknown register with id %hhu\n", id);
					exit(1);
			}
		}

		void set_reg_by_id(uint8_t id, uint64_t value)
		{
			switch (id) {
				case R_0_ID:
					r_0 = value;
					break;
				case R_1_ID:
					r_1 = value;
					break;
				case R_2_ID:
					r_2 = value;
					break;
				case R_3_ID:
					r_3 = value;
					break;
				case R_INSTRUCTION_P_ID:
					r_instruction_p = (uint8_t *) value;
					break;
				case R_STACK_P_ID:
					r_stack_p = (uint8_t *) value;
					break;
				case R_FRAME_P_ID:
					r_frame_p = (uint8_t *) value;
					break;
				case R_ACCUMULATOR_ID:
					r_accumulator = value;
					break;
			}
		}

		void run()
		{
			#ifdef CPU_DUMP_DEBUG
			dump_stack();
			dump_registers();
			#endif

			while (r_instruction_p < memory->location() + program_size) {
				uint16_t instruction = fetch<uint16_t>();

				#ifdef CPU_DUMP_DEBUG
				printf("now executing opcode = %hu\n", instruction);
				#endif

				execute(instruction);

				#ifdef CPU_DUMP_DEBUG
				dump_stack();
				dump_registers();
				#endif
			}
		}

		void dump_stack()
		{
			printf("Stack dump:\n");
			uint8_t *stack_location = memory->location() + program_size;
			memory->dump(stack_location, stack_location + stack_size, r_stack_p);
		}

		void dump_program()
		{
			printf("Program dump:\n");
			uint8_t *program_location = memory->location();
			memory->dump(program_location, program_location + program_size);
		}

		void dump_registers()
		{
			printf("r_instruction_p = %020lu\n", (size_t) r_instruction_p);
			printf("r_stack_p       = %020lu\n", (size_t) r_stack_p);
			printf("r_frame_p       = %020lu\n", (size_t) r_frame_p);
			printf("r_accumulator   = %020lu\n", r_accumulator);
			printf("r_0             = %020lu\n", r_0);
			printf("r_1             = %020lu\n", r_1);
			printf("r_2             = %020lu\n", r_2);
			printf("r_3             = %020lu\n", r_3);
		}

		template <typename intx_t>
		intx_t fetch()
		{
			intx_t instruction = memory->get<intx_t>(r_instruction_p);
			r_instruction_p += sizeof(intx_t);
			return instruction;
		}

		void execute(uint16_t instruction)
		{
			switch (instruction) {
				case PUSH_8:
					memory->set(r_stack_p, fetch<uint8_t>());
					r_stack_p += 1;
					break;

				case PUSH_16:
					memory->set(r_stack_p, fetch<uint16_t>());
					r_stack_p += 2;
					break;

				case PUSH_32:
					memory->set(r_stack_p, fetch<uint32_t>());
					r_stack_p += 4;
					break;

				case PUSH_64:
					memory->set(r_stack_p, fetch<uint64_t>());
					r_stack_p += 8;
					break;

				case PUSH_REG:
					memory->set(r_stack_p, get_reg_by_id(fetch<uint8_t>()));
					r_stack_p += 8;
					break;

				case POP_8:
					r_stack_p -= 1;
					r_0 = memory->get<uint8_t>(r_stack_p);
					break;

				case POP_16:
					r_stack_p -= 2;
					r_0 = memory->get<uint16_t>(r_stack_p);
					break;

				case POP_32:
					r_stack_p -= 4;
					r_0 = memory->get<uint32_t>(r_stack_p);
					break;

				case POP_64:
					r_stack_p -= 8;
					r_0 = memory->get<uint64_t>(r_stack_p);
					break;

				case POP_REG:
					r_stack_p -= 8;
					set_reg_by_id(fetch<uint8_t>(), memory->get<uint64_t>(r_stack_p));
					break;

				case JUMP:
					r_instruction_p = memory->location() + fetch<uint64_t>();
					break;

				case MOVE_LIT_INTO_REG:
				{
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, fetch<uint64_t>());
					break;
				}

				case MOVE_LIT_INTO_MEM:
				{
					uint8_t *address = (uint8_t *) fetch<uint64_t>();
					memory->set(address, fetch<uint64_t>());
					break;
				}

				case MOVE_REG_INTO_REG:
				{
					uint8_t reg_from = fetch<uint8_t>();
					uint8_t reg_to = fetch<uint8_t>();
					set_reg_by_id(reg_to, get_reg_by_id(reg_from));
					break;
				}

				case ADD_REG_INTO_REG:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					set_reg_by_id(reg_id_2,
						get_reg_by_id(reg_id_2) + get_reg_by_id(reg_id_1));
					break;
				}

				case SUBTRACT_REG_FROM_REG:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					set_reg_by_id(reg_id_2,
						get_reg_by_id(reg_id_2) - get_reg_by_id(reg_id_1));
					break;
				}
			}
		}
};

#endif
