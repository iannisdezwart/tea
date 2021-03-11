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

		// General purpose registers

		uint64_t r_0 = 0;
		#define R_0_ID 0
		uint64_t r_1 = 0;
		#define R_1_ID 1
		uint64_t r_2 = 0;
		#define R_2_ID 2
		uint64_t r_3 = 0;
		#define R_3_ID 3

		// Special registers

		uint8_t *r_instruction_p;
		#define R_INSTRUCTION_P_ID 4
		uint8_t *r_stack_p;
		#define R_STACK_P_ID 5
		uint8_t *r_frame_p;
		#define R_FRAME_P_ID 6

		uint64_t r_accumulator = 0;
		#define R_ACCUMULATOR_ID 7

		uint64_t stack_frame_size = 0;

		// Flags

		bool overflow_flag = false;
		bool division_error_flag = false;
		bool equal_flag = false;
		bool greater_flag = false;

		CPU(ProgramBuilder& program, size_t stack_size)
		{
			this->stack_size = stack_size;
			program_size = program.i;
			program.update_label_references();
			memory = program.build(stack_size);

			r_instruction_p = memory->location();
			r_stack_p = r_instruction_p + memory->size - 1;
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
			dump_program();
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
			memory->dump(stack_location, stack_location + stack_size, r_stack_p, r_frame_p);
		}

		void dump_program()
		{
			printf("Program dump:\n");
			uint8_t *program_location = memory->location();
			memory->dump(program_location, program_location + program_size);
		}

		void dump_registers()
		{
			printf("r_instruction_p = %020lu = 0x%016lx\n",
				(size_t) r_instruction_p, (size_t) r_instruction_p);
			printf("r_stack_p       = %020lu = 0x%016lx\n",
				(size_t) r_stack_p, (size_t) r_stack_p);
			printf("r_frame_p       = %020lu = 0x%016lx\n",
				(size_t) r_frame_p, (size_t) r_frame_p);
			printf("r_accumulator   = %020lu = 0x%016lx\n", r_accumulator, r_accumulator);
			printf("r_0             = %020lu = 0x%016lx\n", r_0, r_0);
			printf("r_1             = %020lu = 0x%016lx\n", r_1, r_1);
			printf("r_2             = %020lu = 0x%016lx\n", r_2, r_2);
			printf("r_3             = %020lu = 0x%016lx\n", r_3, r_3);
			printf("greater_flag = %d, equal_flag =  %d\n", greater_flag, equal_flag);
		}

		template <typename intx_t>
		intx_t fetch()
		{
			intx_t instruction = memory->get<intx_t>(r_instruction_p);
			r_instruction_p += sizeof(intx_t);
			return instruction;
		}

		template <typename intx_t>
		void push(intx_t value)
		{
			r_stack_p -= sizeof(intx_t);
			stack_frame_size += sizeof(intx_t);
			memory->set(r_stack_p, value);
		}

		template <typename intx_t>
		intx_t pop()
		{
			intx_t value = memory->get<uint64_t>(r_stack_p);
			r_stack_p += sizeof(intx_t);
			stack_frame_size -= sizeof(intx_t);
			return value;
		}

		void push_stackframe()
		{
			push(get_reg_by_id(R_0_ID));
			push(get_reg_by_id(R_1_ID));
			push(get_reg_by_id(R_2_ID));
			push(get_reg_by_id(R_3_ID));
			push(get_reg_by_id(R_INSTRUCTION_P_ID));
			push(stack_frame_size + 8);

			r_frame_p = r_stack_p;
			stack_frame_size = 0;
		}

		void pop_stackframe()
		{
			r_stack_p = r_frame_p;

			stack_frame_size = pop<uint64_t>();
			uint64_t saved_stack_frame_size = stack_frame_size;

			r_instruction_p = (uint8_t *) pop<uint64_t>();
			r_3 = pop<uint64_t>();
			r_2 = pop<uint64_t>();
			r_1 = pop<uint64_t>();
			r_0 = pop<uint64_t>();

			uint64_t arguments_size = pop<uint64_t>();

			r_frame_p += saved_stack_frame_size;
			r_stack_p += arguments_size;
		}

		void execute(uint16_t instruction)
		{
			switch (instruction) {
				case MOVE_8_INTO_REG:
				{
					uint64_t lit = fetch<uint8_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, lit);
					break;
				}

				case MOVE_16_INTO_REG:
				{
					uint64_t lit = fetch<uint16_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, lit);
					break;
				}

				case MOVE_32_INTO_REG:
				{
					uint64_t lit = fetch<uint32_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, lit);
					break;
				}

				case MOVE_64_INTO_REG:
				{
					uint64_t lit = fetch<uint64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, lit);
					break;
				}

				case MOVE_8_INTO_MEM:
				{
					uint64_t lit = fetch<uint8_t>();
					uint8_t *address = (uint8_t *) fetch<uint64_t>();
					memory->set(address, lit);
					break;
				}

				case MOVE_16_INTO_MEM:
				{
					uint64_t lit = fetch<uint16_t>();
					uint8_t *address = (uint8_t *) fetch<uint64_t>();
					memory->set(address, lit);
					break;
				}

				case MOVE_32_INTO_MEM:
				{
					uint64_t lit = fetch<uint32_t>();
					uint8_t *address = (uint8_t *) fetch<uint64_t>();
					memory->set(address, lit);
					break;
				}

				case MOVE_64_INTO_MEM:
				{
					uint64_t lit = fetch<uint64_t>();
					uint8_t *address = (uint8_t *) fetch<uint64_t>();
					memory->set(address, lit);
					break;
				}

				case MOVE_REG_INTO_REG:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					set_reg_by_id(reg_id_2, get_reg_by_id(reg_id_1));
					break;
				}

				case MOVE_REG_INTO_MEM:
				{
					uint8_t reg_id = fetch<uint8_t>();
					uint8_t *address = (uint8_t *) fetch<uint64_t>();
					memory->set(address, get_reg_by_id(reg_id));
					break;
				}

				case MOVE_MEM_INTO_REG:
				{
					uint8_t *address = (uint8_t *) fetch<uint64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, memory->get<uint64_t>(address));
					break;
				}

				case MOVE_FRAME_OFFSET_8_INTO_REG:
				{
					int64_t offset = fetch<int64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					uint8_t value = *(r_frame_p + offset);
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_FRAME_OFFSET_16_INTO_REG:
				{
					int64_t offset = fetch<int64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					uint16_t value = *(r_frame_p + offset);
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_FRAME_OFFSET_32_INTO_REG:
				{
					int64_t offset = fetch<int64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					uint32_t value = *(r_frame_p + offset);
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_FRAME_OFFSET_64_INTO_REG:
				{
					int64_t offset = fetch<int64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					uint64_t value = *(r_frame_p + offset);
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_REG_INTO_FRAME_OFFSET:
				{
					uint8_t reg_id = fetch<uint8_t>();
					int64_t offset = fetch<uint64_t>();
					uint64_t value = get_reg_by_id(reg_id);
					*(r_frame_p + offset) = value;
					break;
				}

				case ADD_8_INTO_REG:
				{
					uint64_t lit = fetch<uint8_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) + lit);
					break;
				}

				case ADD_16_INTO_REG:
				{
					uint64_t lit = fetch<uint16_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) + lit);
					break;
				}

				case ADD_32_INTO_REG:
				{
					uint64_t lit = fetch<uint32_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) + lit);
					break;
				}

				case ADD_64_INTO_REG:
				{
					uint64_t lit = fetch<uint64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) + lit);
					break;
				}

				case ADD_REG_INTO_REG:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					set_reg_by_id(reg_id_2, get_reg_by_id(reg_id_2) + get_reg_by_id(reg_id_1));
					break;
				}

				case SUBTRACT_8_FROM_REG:
				{
					uint64_t lit = fetch<uint8_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) - lit);
					break;
				}

				case SUBTRACT_16_FROM_REG:
				{
					uint64_t lit = fetch<uint16_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) - lit);
					break;
				}

				case SUBTRACT_32_FROM_REG:
				{
					uint64_t lit = fetch<uint32_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) - lit);
					break;
				}

				case SUBTRACT_64_FROM_REG:
				{
					uint64_t lit = fetch<uint64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) - lit);
					break;
				}

				case SUBTRACT_REG_FROM_REG:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					set_reg_by_id(reg_id_2, get_reg_by_id(reg_id_2) - get_reg_by_id(reg_id_1));
					break;
				}

				case MULTIPLY_8_INTO_REG:
				{
					uint64_t lit = fetch<uint8_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) * lit);
					break;
				}

				case MULTIPLY_16_INTO_REG:
				{
					uint64_t lit = fetch<uint16_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) * lit);
					break;
				}

				case MULTIPLY_32_INTO_REG:
				{
					uint64_t lit = fetch<uint32_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) * lit);
					break;
				}

				case MULTIPLY_64_INTO_REG:
				{
					uint64_t lit = fetch<uint64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) * lit);
					break;
				}

				case MULTIPLY_REG_INTO_REG:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					set_reg_by_id(reg_id_2, get_reg_by_id(reg_id_2) * get_reg_by_id(reg_id_1));
					break;
				}

				case DIVIDE_8_FROM_REG:
				{
					uint64_t lit = fetch<uint8_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) - lit);
					break;
				}

				case DIVIDE_16_FROM_REG:
				{
					uint64_t lit = fetch<uint16_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) / lit);
					break;
				}

				case DIVIDE_32_FROM_REG:
				{
					uint64_t lit = fetch<uint32_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) / lit);
					break;
				}

				case DIVIDE_64_FROM_REG:
				{
					uint64_t lit = fetch<uint64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) / lit);
					break;
				}

				case DIVIDE_REG_FROM_REG:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					set_reg_by_id(reg_id_2, get_reg_by_id(reg_id_2) / get_reg_by_id(reg_id_1));
					break;
				}

				case AND_8_INTO_REG:
				{
					uint64_t lit = fetch<uint8_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) & lit);
					break;
				}

				case AND_16_INTO_REG:
				{
					uint64_t lit = fetch<uint16_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) & lit);
					break;
				}

				case AND_32_INTO_REG:
				{
					uint64_t lit = fetch<uint32_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) & lit);
					break;
				}

				case AND_64_INTO_REG:
				{
					uint64_t lit = fetch<uint64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) & lit);
					break;
				}

				case AND_REG_INTO_REG:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					set_reg_by_id(reg_id_2, get_reg_by_id(reg_id_2) & get_reg_by_id(reg_id_1));
					break;
				}

				case OR_8_INTO_REG:
				{
					uint64_t lit = fetch<uint8_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) | lit);
					break;
				}

				case OR_16_INTO_REG:
				{
					uint64_t lit = fetch<uint16_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) | lit);
					break;
				}

				case OR_32_INTO_REG:
				{
					uint64_t lit = fetch<uint32_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) | lit);
					break;
				}

				case OR_64_INTO_REG:
				{
					uint64_t lit = fetch<uint64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) | lit);
					break;
				}

				case OR_REG_INTO_REG:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					set_reg_by_id(reg_id_2, get_reg_by_id(reg_id_2) | get_reg_by_id(reg_id_1));
					break;
				}

				case XOR_8_INTO_REG:
				{
					uint64_t lit = fetch<uint8_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) ^ lit);
					break;
				}

				case XOR_16_INTO_REG:
				{
					uint64_t lit = fetch<uint16_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) ^ lit);
					break;
				}

				case XOR_32_INTO_REG:
				{
					uint64_t lit = fetch<uint32_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) ^ lit);
					break;
				}

				case XOR_64_INTO_REG:
				{
					uint64_t lit = fetch<uint64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) ^ lit);
					break;
				}

				case XOR_REG_INTO_REG:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					set_reg_by_id(reg_id_2, get_reg_by_id(reg_id_2) ^ get_reg_by_id(reg_id_1));
					break;
				}

				case LEFT_SHIFT_REG_BY_8:
				{
					uint8_t reg_id = fetch<uint8_t>();
					uint8_t lit = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) << lit);
					break;
				}

				case LEFT_SHIFT_REG_BY_REG:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					set_reg_by_id(reg_id_1, get_reg_by_id(reg_id_1) << get_reg_by_id(reg_id_2));
					break;
				}

				case RIGHT_SHIFT_REG_BY_8:
				{
					uint8_t reg_id = fetch<uint8_t>();
					uint8_t lit = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) >> lit);
					break;
				}

				case RIGHT_SHIFT_REG_BY_REG:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					set_reg_by_id(reg_id_1, get_reg_by_id(reg_id_1) >> get_reg_by_id(reg_id_2));
					break;
				}

				case INCREMENT_REG:
				{
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) + 1);
					break;
				}

				case DECREMENT_REG:
				{
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) - 1);
					break;
				}

				case NOT_REG:
				{
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, !get_reg_by_id(reg_id));
					break;
				}

				case COMPARE_8_TO_REG:
				{
					uint64_t lit = fetch<uint8_t>();
					uint8_t reg_id = fetch<uint8_t>();
					uint64_t reg_value = get_reg_by_id(reg_id);

					if (lit > reg_value) {
						greater_flag = true;
						equal_flag = false;
					} else if (lit == reg_value) {
						greater_flag = false;
						equal_flag = true;
					} else {
						greater_flag = false;
						equal_flag = false;
					}

					break;
				}

				case COMPARE_16_TO_REG:
				{
					uint64_t lit = fetch<uint16_t>();
					uint8_t reg_id = fetch<uint8_t>();
					uint64_t reg_value = get_reg_by_id(reg_id);

					if (lit > reg_value) {
						greater_flag = true;
						equal_flag = false;
					} else if (lit == reg_value) {
						greater_flag = false;
						equal_flag = true;
					} else {
						greater_flag = false;
						equal_flag = false;
					}

					break;
				}

				case COMPARE_32_TO_REG:
				{
					uint64_t lit = fetch<uint32_t>();
					uint8_t reg_id = fetch<uint8_t>();
					uint64_t reg_value = get_reg_by_id(reg_id);

					if (lit > reg_value) {
						greater_flag = true;
						equal_flag = false;
					} else if (lit == reg_value) {
						greater_flag = false;
						equal_flag = true;
					} else {
						greater_flag = false;
						equal_flag = false;
					}

					break;
				}

				case COMPARE_64_TO_REG:
				{
					uint64_t lit = fetch<uint64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					uint64_t reg_value = get_reg_by_id(reg_id);

					if (lit > reg_value) {
						greater_flag = true;
						equal_flag = false;
					} else if (lit == reg_value) {
						greater_flag = false;
						equal_flag = true;
					} else {
						greater_flag = false;
						equal_flag = false;
					}

					break;
				}

				case COMPARE_REG_TO_8:
				{
					uint8_t reg_id = fetch<uint8_t>();
					uint64_t reg_value = get_reg_by_id(reg_id);
					uint64_t lit = fetch<uint8_t>();

					if (reg_value > lit) {
						greater_flag = true;
						equal_flag = false;
					} else if (reg_value == lit) {
						greater_flag = false;
						equal_flag = true;
					} else {
						greater_flag = false;
						equal_flag = false;
					}

					break;
				}

				case COMPARE_REG_TO_16:
				{
					uint8_t reg_id = fetch<uint8_t>();
					uint64_t reg_value = get_reg_by_id(reg_id);
					uint64_t lit = fetch<uint16_t>();

					if (reg_value > lit) {
						greater_flag = true;
						equal_flag = false;
					} else if (reg_value == lit) {
						greater_flag = false;
						equal_flag = true;
					} else {
						greater_flag = false;
						equal_flag = false;
					}

					break;
				}

				case COMPARE_REG_TO_32:
				{
					uint8_t reg_id = fetch<uint8_t>();
					uint64_t reg_value = get_reg_by_id(reg_id);
					uint64_t lit = fetch<uint32_t>();

					if (reg_value > lit) {
						greater_flag = true;
						equal_flag = false;
					} else if (reg_value == lit) {
						greater_flag = false;
						equal_flag = true;
					} else {
						greater_flag = false;
						equal_flag = false;
					}

					break;
				}

				case COMPARE_REG_TO_64:
				{
					uint8_t reg_id = fetch<uint8_t>();
					uint64_t reg_value = get_reg_by_id(reg_id);
					uint64_t lit = fetch<uint64_t>();

					if (reg_value > lit) {
						greater_flag = true;
						equal_flag = false;
					} else if (reg_value == lit) {
						greater_flag = false;
						equal_flag = true;
					} else {
						greater_flag = false;
						equal_flag = false;
					}

					break;
				}

				case COMPARE_REG_TO_REG:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint64_t reg_value_1 = get_reg_by_id(reg_id_1);
					uint8_t reg_id_2 = fetch<uint8_t>();
					uint64_t reg_value_2 = get_reg_by_id(reg_id_2);

					if (reg_value_1 > reg_value_2) {
						greater_flag = true;
						equal_flag = false;
					} else if (reg_value_1 == reg_value_2) {
						greater_flag = false;
						equal_flag = true;
					} else {
						greater_flag = false;
						equal_flag = false;
					}

					break;
				}

				case JUMP:
				{
					uint64_t offset = fetch<uint64_t>();
					r_instruction_p = memory->location() + offset;
					break;
				}

				case JUMP_IF_GREATER:
				{
					uint64_t offset = fetch<uint64_t>();
					if (greater_flag) r_instruction_p = memory->location() + offset;
					break;
				}

				case JUMP_IF_GREATER_OR_EQUAL:
				{
					uint64_t offset = fetch<uint64_t>();
					if (greater_flag | equal_flag) r_instruction_p = memory->location() + offset;
					break;
				}

				case JUMP_IF_LESS:
				{
					uint64_t offset = fetch<uint64_t>();
					if (!greater_flag & !equal_flag) r_instruction_p = memory->location() + offset;
					break;
				}

				case JUMP_IF_LESS_OR_EQUAL:
				{
					uint64_t offset = fetch<uint64_t>();
					if (!greater_flag) r_instruction_p = memory->location() + offset;
					break;
				}

				case JUMP_IF_EQUAL:
				{
					uint64_t offset = fetch<uint64_t>();
					if (equal_flag) r_instruction_p = memory->location() + offset;
					break;
				}

				case PUSH_8:
				{
					uint8_t lit = fetch<uint8_t>();
					memory->set(r_stack_p, lit);
					r_stack_p += 1;
					break;
				}

				case PUSH_16:
				{
					uint16_t lit = fetch<uint16_t>();
					memory->set(r_stack_p, lit);
					r_stack_p += 2;
					break;
				}

				case PUSH_32:
				{
					uint32_t lit = fetch<uint32_t>();
					memory->set(r_stack_p, lit);
					r_stack_p += 4;
					break;
				}

				case PUSH_64:
				{
					uint64_t lit = fetch<uint64_t>();
					memory->set(r_stack_p, lit);
					r_stack_p += 8;
					break;
				}

				case PUSH_REG:
				{
					uint8_t reg_id = fetch<uint8_t>();
					memory->set(r_stack_p, get_reg_by_id(reg_id));
					r_stack_p += 8;
					break;
				}

				case POP_8_INTO_REG:
				{
					uint8_t reg_id = fetch<uint8_t>();
					r_stack_p -= 1;
					uint8_t value = memory->get<uint8_t>(r_stack_p);
					set_reg_by_id(reg_id, value);
					break;
				}

				case POP_16_INTO_REG:
				{
					uint8_t reg_id = fetch<uint8_t>();
					r_stack_p -= 2;
					uint16_t value = memory->get<uint16_t>(r_stack_p);
					set_reg_by_id(reg_id, value);
					break;
				}

				case POP_32_INTO_REG:
				{
					uint8_t reg_id = fetch<uint8_t>();
					r_stack_p -= 4;
					uint32_t value = memory->get<uint32_t>(r_stack_p);
					set_reg_by_id(reg_id, value);
					break;
				}

				case POP_64_INTO_REG:
				{
					uint8_t reg_id = fetch<uint8_t>();
					r_stack_p -= 8;
					uint64_t value = memory->get<uint64_t>(r_stack_p);
					set_reg_by_id(reg_id, value);
					break;
				}

				case CALL:
				{
					uint64_t offset = fetch<uint64_t>();
					push_stackframe();
					r_instruction_p = memory->location() + offset;
					break;
				}

				case RETURN:
				{
					pop_stackframe();
					break;
				}

				case LOG_REG:
				{
					uint8_t reg_id = fetch<uint8_t>();
					uint64_t value = get_reg_by_id(reg_id);
					printf("> %lu\n", value);
					break;
				}
			}
		}
};

#endif
