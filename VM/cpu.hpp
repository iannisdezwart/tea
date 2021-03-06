#ifndef TEA_CPU_HEADER
#define TEA_CPU_HEADER

#include <bits/stdc++.h>

#include "memory-mapper.hpp"
#include "ram-device.hpp"
#include "io-device.hpp"
#include "../Assembler/executable.hpp"
#include "../Assembler/byte_code.hpp"

using namespace std;

class CPU {
	public:
		MemoryMapper memory_mapper;

		// Memory devices

		IODevice io_device;
		RamDevice program_ram_device;

		// Program segment sizes

		size_t static_data_size;
		size_t program_size;
		size_t stack_size;

		// General purpose registers

		static constexpr size_t general_purpose_register_count = 4;

		uint64_t r_0 = 0;
		#define R_0_ID 0
		uint64_t r_1 = 0;
		#define R_1_ID 1
		uint64_t r_2 = 0;
		#define R_2_ID 2
		uint64_t r_3 = 0;
		#define R_3_ID 3

		// Special registers

		uint64_t r_instruction_p;
		#define R_INSTRUCTION_P_ID 4
		uint64_t r_stack_p;
		#define R_STACK_P_ID 5
		uint64_t r_frame_p;
		#define R_FRAME_P_ID 6
		uint64_t r_ret = 0;
		#define R_RET_ID 7

		uint64_t current_stack_frame_size = 0;

		static const char *reg_to_str(uint8_t reg_id)
		{
			switch (reg_id) {
				default:
					return "UNDEFINED";

				case R_0_ID:
					return "R_0";

				case R_1_ID:
					return "R_1";

				case R_2_ID:
					return "R_2";

				case R_3_ID:
					return "R_3";

				case R_INSTRUCTION_P_ID:
					return "R_INS_P";

				case R_STACK_P_ID:
					return "R_STACK_P";

				case R_FRAME_P_ID:
					return "R_FRAME_P";

				case R_RET_ID:
					return "R_RET";
			}
		}

		// Flags

		bool overflow_flag = false;
		bool division_error_flag = false;
		bool equal_flag = false;
		bool greater_flag = false;

		CPU(Executable& executable, size_t stack_size)
			: stack_size(stack_size),
				static_data_size(executable.static_data_size),
				program_size(executable.program_size),
				io_device(IO_DEVICE_OFFSET),
				program_ram_device(PROGRAM_START,
					PROGRAM_START + executable.size + stack_size)
		{
			memory_mapper.add_device(&io_device);
			memory_mapper.add_device(&program_ram_device);
			program_ram_device.copy_from(executable.data, executable.size);

			r_instruction_p = program_location();
			r_stack_p = stack_top();
			r_frame_p = r_stack_p;
		}

		static constexpr const ssize_t stack_frame_size = 48;

		uint64_t static_data_location()
		{
			return PROGRAM_START;
		}

		uint64_t program_location()
		{
			return PROGRAM_START + static_data_size;
		}

		uint64_t stack_bottom()
		{
			return PROGRAM_START + static_data_size + program_size + stack_size;
		}

		uint64_t stack_top()
		{
			return PROGRAM_START + static_data_size + program_size;
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
					return r_instruction_p;
				case R_STACK_P_ID:
					return r_stack_p;
				case R_FRAME_P_ID:
					return r_frame_p;
				case R_RET_ID:
					return r_ret;
				default:
					throw "Unknown register with id " + to_string(id);
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
					r_instruction_p = value;
					break;
				case R_STACK_P_ID:
					r_stack_p = value;
					break;
				case R_FRAME_P_ID:
					r_frame_p = value;
					break;
				case R_RET_ID:
					r_ret = value;
					break;
				default:
					throw "Unknown register with id " + to_string(id);
			}
		}

		Instruction step()
		{
			#ifdef RESTORE_INSTRUCTION_POINTER_ON_THROW
			uint64_t prev_r_instruction_p = r_instruction_p;
			#endif

			Instruction instruction = (Instruction) fetch<uint16_t>();

			#ifdef RESTORE_INSTRUCTION_POINTER_ON_THROW
			try {
				execute(instruction);
			} catch (const string& err_message) {
				r_instruction_p = prev_r_instruction_p;
				throw err_message;
			}
			#else
			execute(instruction);
			#endif

			return instruction;
		}

		void run()
		{
			while (r_instruction_p < stack_top()) {
				step();
			}
		}

	private:
		template <typename intx_t>
		intx_t fetch()
		{
			intx_t instruction = memory_mapper.get<intx_t>(r_instruction_p);
			r_instruction_p += sizeof(intx_t);
			return instruction;
		}

		template <typename intx_t>
		void push(intx_t value)
		{
			memory_mapper.set(r_stack_p, value);
			r_stack_p += sizeof(intx_t);
			current_stack_frame_size += sizeof(intx_t);
		}

		template <typename intx_t>
		intx_t pop()
		{
			r_stack_p -= sizeof(intx_t);
			intx_t value = memory_mapper.get<uint64_t>(r_stack_p);
			current_stack_frame_size -= sizeof(intx_t);
			return value;
		}

		void push_stack_frame()
		{
			push(get_reg_by_id(R_0_ID));
			push(get_reg_by_id(R_1_ID));
			push(get_reg_by_id(R_2_ID));
			push(get_reg_by_id(R_3_ID));
			push(get_reg_by_id(R_INSTRUCTION_P_ID));
			push(current_stack_frame_size + 8);

			r_frame_p = r_stack_p;
			current_stack_frame_size = 0;
		}

		void pop_stack_frame()
		{
			r_stack_p = r_frame_p;

			current_stack_frame_size = pop<uint64_t>();
			uint64_t saved_stack_frame_size = current_stack_frame_size;

			r_instruction_p = pop<uint64_t>();
			r_3 = pop<uint64_t>();
			r_2 = pop<uint64_t>();
			r_1 = pop<uint64_t>();
			r_0 = pop<uint64_t>();

			uint64_t arguments_size = pop<uint64_t>();
			current_stack_frame_size -= arguments_size + 8;

			r_frame_p -= saved_stack_frame_size;
			r_stack_p -= arguments_size;
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
					uint64_t address = fetch<uint64_t>();
					memory_mapper.set(address, lit);
					break;
				}

				case MOVE_16_INTO_MEM:
				{
					uint64_t lit = fetch<uint16_t>();
					uint64_t address = fetch<uint64_t>();
					memory_mapper.set(address, lit);
					break;
				}

				case MOVE_32_INTO_MEM:
				{
					uint64_t lit = fetch<uint32_t>();
					uint64_t address = fetch<uint64_t>();
					memory_mapper.set(address, lit);
					break;
				}

				case MOVE_64_INTO_MEM:
				{
					uint64_t lit = fetch<uint64_t>();
					uint64_t address = fetch<uint64_t>();
					memory_mapper.set(address, lit);
					break;
				}

				case MOVE_REG_INTO_REG:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					uint64_t value = get_reg_by_id(reg_id_1);
					set_reg_by_id(reg_id_2, value);
					break;
				}

				case MOVE_REG_INTO_MEM_8:
				{
					uint8_t reg_id = fetch<uint8_t>();
					uint64_t address = fetch<uint64_t>();
					uint8_t value = get_reg_by_id(reg_id) & 0xFF;
					memory_mapper.set(address, value);
					break;
				}

				case MOVE_REG_INTO_MEM_16:
				{
					uint8_t reg_id = fetch<uint8_t>();
					uint64_t address = fetch<uint64_t>();
					uint16_t value = get_reg_by_id(reg_id) & 0xFFFF;
					memory_mapper.set(address, value);
					break;
				}

				case MOVE_REG_INTO_MEM_32:
				{
					uint8_t reg_id = fetch<uint8_t>();
					uint64_t address = fetch<uint64_t>();
					uint32_t value = get_reg_by_id(reg_id) & 0xFFFFFFFF;
					memory_mapper.set(address, value);
					break;
				}

				case MOVE_REG_INTO_MEM_64:
				{
					uint8_t reg_id = fetch<uint8_t>();
					uint64_t address = fetch<uint64_t>();
					uint64_t value = get_reg_by_id(reg_id);
					memory_mapper.set(address, value);
					break;
				}

				case MOVE_MEM_8_INTO_REG:
				{
					uint64_t address = fetch<uint64_t>();
					uint8_t value = memory_mapper.get<uint8_t>(address);
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_MEM_16_INTO_REG:
				{
					uint64_t address = fetch<uint64_t>();
					uint16_t value = memory_mapper.get<uint16_t>(address);
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_MEM_32_INTO_REG:
				{
					uint64_t address = fetch<uint64_t>();
					uint32_t value = memory_mapper.get<uint32_t>(address);
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_MEM_64_INTO_REG:
				{
					uint64_t address = fetch<uint64_t>();
					uint64_t value = memory_mapper.get<uint64_t>(address);
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_REG_POINTER_8_INTO_REG:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					uint64_t address = get_reg_by_id(reg_id_1);
					uint8_t value = memory_mapper.get<uint8_t>(address);
					set_reg_by_id(reg_id_2, value);
					break;
				}

				case MOVE_REG_POINTER_16_INTO_REG:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					uint64_t address = get_reg_by_id(reg_id_1);
					uint16_t value = memory_mapper.get<uint16_t>(address);
					set_reg_by_id(reg_id_2, value);
					break;
				}

				case MOVE_REG_POINTER_32_INTO_REG:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					uint64_t address = get_reg_by_id(reg_id_1);
					uint32_t value = memory_mapper.get<uint32_t>(address);
					set_reg_by_id(reg_id_2, value);
					break;
				}

				case MOVE_REG_POINTER_64_INTO_REG:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					uint64_t address = get_reg_by_id(reg_id_1);
					uint64_t value = memory_mapper.get<uint64_t>(address);
					set_reg_by_id(reg_id_2, value);
					break;
				}

				case MOVE_REG_INTO_REG_POINTER_8:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					uint64_t address = get_reg_by_id(reg_id_2);
					uint8_t value = get_reg_by_id(reg_id_1);
					memory_mapper.set<uint8_t>(address, value);
					break;
				}

				case MOVE_REG_INTO_REG_POINTER_16:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					uint64_t address = get_reg_by_id(reg_id_2);
					uint16_t value = get_reg_by_id(reg_id_1);
					memory_mapper.set<uint16_t>(address, value);
					break;
				}

				case MOVE_REG_INTO_REG_POINTER_32:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					uint64_t address = get_reg_by_id(reg_id_2);
					uint32_t value = get_reg_by_id(reg_id_1);
					memory_mapper.set<uint32_t>(address, value);
					break;
				}

				case MOVE_REG_INTO_REG_POINTER_64:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					uint64_t address = get_reg_by_id(reg_id_2);
					uint64_t value = get_reg_by_id(reg_id_1);
					memory_mapper.set<uint64_t>(address, value);
					break;
				}

				case MOVE_FRAME_OFFSET_8_INTO_REG:
				{
					int64_t offset = fetch<int64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					uint8_t value = memory_mapper.get<uint8_t>(r_frame_p + offset);
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_FRAME_OFFSET_16_INTO_REG:
				{
					int64_t offset = fetch<int64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					uint16_t value = memory_mapper.get<uint16_t>(r_frame_p + offset);
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_FRAME_OFFSET_32_INTO_REG:
				{
					int64_t offset = fetch<int64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					uint32_t value = memory_mapper.get<uint32_t>(r_frame_p + offset);
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_FRAME_OFFSET_64_INTO_REG:
				{
					int64_t offset = fetch<int64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					uint64_t value = memory_mapper.get<uint64_t>(r_frame_p + offset);
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_REG_INTO_FRAME_OFFSET_8:
				{
					uint8_t reg_id = fetch<uint8_t>();
					int64_t offset = fetch<uint64_t>();
					uint8_t value = get_reg_by_id(reg_id) & 0xFF;
					memory_mapper.set(r_frame_p + offset, value);
					break;
				}

				case MOVE_REG_INTO_FRAME_OFFSET_16:
				{
					uint8_t reg_id = fetch<uint8_t>();
					int64_t offset = fetch<uint64_t>();
					uint16_t value = get_reg_by_id(reg_id) & 0xFFFF;
					memory_mapper.set(r_frame_p + offset, value);
					break;
				}

				case MOVE_REG_INTO_FRAME_OFFSET_32:
				{
					uint8_t reg_id = fetch<uint8_t>();
					int64_t offset = fetch<uint64_t>();
					uint32_t value = get_reg_by_id(reg_id) & 0xFFFFFFFF;
					memory_mapper.set(r_frame_p + offset, value);
					break;
				}

				case MOVE_REG_INTO_FRAME_OFFSET_64:
				{
					uint8_t reg_id = fetch<uint8_t>();
					int64_t offset = fetch<uint64_t>();
					uint64_t value = get_reg_by_id(reg_id);
					memory_mapper.set(r_frame_p + offset, value);
					break;
				}

				case MOVE_STACK_TOP_OFFSET_8_INTO_REG:
				{
					int64_t offset = fetch<int64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					uint8_t value = memory_mapper.get<uint8_t>(stack_top() + offset);
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_STACK_TOP_OFFSET_16_INTO_REG:
				{
					int64_t offset = fetch<int64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					uint16_t value = memory_mapper.get<uint16_t>(stack_top() + offset);
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_STACK_TOP_OFFSET_32_INTO_REG:
				{
					int64_t offset = fetch<int64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					uint32_t value = memory_mapper.get<uint32_t>(stack_top() + offset);
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_STACK_TOP_OFFSET_64_INTO_REG:
				{
					int64_t offset = fetch<int64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					uint64_t value = memory_mapper.get<uint64_t>(stack_top() + offset);
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_REG_INTO_STACK_TOP_OFFSET_8:
				{
					uint8_t reg_id = fetch<uint8_t>();
					int64_t offset = fetch<int64_t>();
					uint8_t value = get_reg_by_id(reg_id) & 0xFF;
					memory_mapper.set(stack_top() + offset, value);
					break;
				}

				case MOVE_REG_INTO_STACK_TOP_OFFSET_16:
				{
					uint8_t reg_id = fetch<uint8_t>();
					int64_t offset = fetch<int64_t>();
					uint16_t value = get_reg_by_id(reg_id) & 0xFFFF;
					memory_mapper.set(stack_top() + offset, value);
					break;
				}

				case MOVE_REG_INTO_STACK_TOP_OFFSET_32:
				{
					uint8_t reg_id = fetch<uint8_t>();
					int64_t offset = fetch<int64_t>();
					uint32_t value = get_reg_by_id(reg_id) & 0xFFFFFFFF;
					memory_mapper.set(stack_top() + offset, value);
					break;
				}

				case MOVE_REG_INTO_STACK_TOP_OFFSET_64:
				{
					uint8_t reg_id = fetch<uint8_t>();
					int64_t offset = fetch<int64_t>();
					uint64_t value = get_reg_by_id(reg_id);
					memory_mapper.set(stack_top() + offset, value);
					break;
				}

				case MOVE_STACK_TOP_ADDRESS_INTO_REG:
				{
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, stack_top());
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
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) / lit);
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

				case TAKE_MODULO_8_OF_REG:
				{
					uint8_t lit = fetch<uint8_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) % lit);
					break;
				}

				case TAKE_MODULO_16_OF_REG:
				{
					uint16_t lit = fetch<uint16_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) % lit);
					break;
				}

				case TAKE_MODULO_32_OF_REG:
				{
					uint32_t lit = fetch<uint32_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) % lit);
					break;
				}

				case TAKE_MODULO_64_OF_REG:
				{
					uint64_t lit = fetch<uint64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, get_reg_by_id(reg_id) % lit);
					break;
				}

				case TAKE_MODULO_REG_OF_REG:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					set_reg_by_id(reg_id_2, get_reg_by_id(reg_id_2) % get_reg_by_id(reg_id_1));
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
					set_reg_by_id(reg_id, ~get_reg_by_id(reg_id));
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

				case SET_REG_IF_GREATER:
				{
					uint8_t reg_id = fetch<uint8_t>();
					if (greater_flag) set_reg_by_id(reg_id, 1);
					else set_reg_by_id(reg_id, 0);
					break;
				}

				case SET_REG_IF_GREATER_OR_EQUAL:
				{
					uint8_t reg_id = fetch<uint8_t>();
					if (greater_flag | equal_flag) set_reg_by_id(reg_id, 1);
					else set_reg_by_id(reg_id, 0);
					break;
				}

				case SET_REG_IF_LESS:
				{
					uint8_t reg_id = fetch<uint8_t>();
					if (!greater_flag & !equal_flag) set_reg_by_id(reg_id, 1);
					else set_reg_by_id(reg_id, 0);
					break;
				}

				case SET_REG_IF_LESS_OR_EQUAL:
				{
					uint8_t reg_id = fetch<uint8_t>();
					if (!greater_flag) set_reg_by_id(reg_id, 1);
					else set_reg_by_id(reg_id, 0);
					break;
				}

				case SET_REG_IF_EQUAL:
				{
					uint8_t reg_id = fetch<uint8_t>();
					if (equal_flag) set_reg_by_id(reg_id, 1);
					else set_reg_by_id(reg_id, 0);
					break;
				}

				case SET_REG_IF_NOT_EQUAL:
				{
					uint8_t reg_id = fetch<uint8_t>();
					if (!equal_flag) set_reg_by_id(reg_id, 1);
					else set_reg_by_id(reg_id, 0);
					break;
				}

				case JUMP:
				{
					uint64_t address = fetch<uint64_t>();
					r_instruction_p = address;
					break;
				}

				case JUMP_IF_GREATER:
				{
					uint64_t address = fetch<uint64_t>();
					if (greater_flag) r_instruction_p = address;
					break;
				}

				case JUMP_IF_GREATER_OR_EQUAL:
				{
					uint64_t address = fetch<uint64_t>();
					if (greater_flag | equal_flag) r_instruction_p = address;
					break;
				}

				case JUMP_IF_LESS:
				{
					uint64_t address = fetch<uint64_t>();
					if (!greater_flag & !equal_flag) r_instruction_p = address;
					break;
				}

				case JUMP_IF_LESS_OR_EQUAL:
				{
					uint64_t address = fetch<uint64_t>();
					if (!greater_flag) r_instruction_p = address;
					break;
				}

				case JUMP_IF_EQUAL:
				{
					uint64_t address = fetch<uint64_t>();
					if (equal_flag) r_instruction_p = address;
					break;
				}

				case JUMP_IF_NOT_EQUAL:
				{
					uint64_t address = fetch<uint64_t>();
					if (!equal_flag) r_instruction_p = address;
					break;
				}

				case PUSH_8:
				{
					uint8_t lit = fetch<uint8_t>();
					push(lit);
					break;
				}

				case PUSH_16:
				{
					uint16_t lit = fetch<uint16_t>();
					push(lit);
					break;
				}

				case PUSH_32:
				{
					uint32_t lit = fetch<uint32_t>();
					push(lit);
					break;
				}

				case PUSH_64:
				{
					uint64_t lit = fetch<uint64_t>();
					push(lit);
					break;
				}

				case PUSH_REG_8:
				{
					uint8_t reg_id = fetch<uint8_t>();
					uint8_t value = get_reg_by_id(reg_id) & 0xFF;
					push(value);
					break;
				}

				case PUSH_REG_16:
				{
					uint8_t reg_id = fetch<uint8_t>();
					uint16_t value = get_reg_by_id(reg_id) & 0xFFFF;
					push(value);
					break;
				}

				case PUSH_REG_32:
				{
					uint8_t reg_id = fetch<uint8_t>();
					uint32_t value = get_reg_by_id(reg_id) & 0xFFFFFFFF;
					push(value);
					break;
				}

				case PUSH_REG_64:
				{
					uint8_t reg_id = fetch<uint8_t>();
					uint64_t value = get_reg_by_id(reg_id);
					push(value);
					break;
				}

				case POP_8_INTO_REG:
				{
					uint8_t reg_id = fetch<uint8_t>();
					uint8_t value = pop<uint8_t>();
					set_reg_by_id(reg_id, value);
					break;
				}

				case POP_16_INTO_REG:
				{
					uint8_t reg_id = fetch<uint8_t>();
					uint16_t value = pop<uint16_t>();
					set_reg_by_id(reg_id, value);
					break;
				}

				case POP_32_INTO_REG:
				{
					uint8_t reg_id = fetch<uint8_t>();
					uint32_t value = pop<uint32_t>();
					set_reg_by_id(reg_id, value);
					break;
				}

				case POP_64_INTO_REG:
				{
					uint8_t reg_id = fetch<uint8_t>();
					uint64_t value = pop<uint64_t>();
					set_reg_by_id(reg_id, value);
					break;
				}

				case CALL:
				{
					uint64_t address = fetch<uint64_t>();
					push_stack_frame();
					r_instruction_p = address;
					break;
				}

				case RETURN:
				{
					pop_stack_frame();
					break;
				}

				case ALLOCATE_STACK:
				{
					uint64_t size = fetch<uint64_t>();
					r_stack_p += size;
					current_stack_frame_size += size;
					break;
				}

				case DEALLOCATE_STACK:
				{
					uint64_t size = fetch<uint64_t>();
					r_stack_p -= size;
					current_stack_frame_size -= size;
					break;
				}

				case LOG_REG:
				{
					uint8_t reg_id = fetch<uint8_t>();
					uint64_t value = get_reg_by_id(reg_id);
					printf(">>> REG_%hhu = 0x%016lx = %020lu\n", reg_id, value, value);
					break;
				}

				case COMMENT:
				case LABEL:
				{
					while (fetch<uint8_t>() != '\0');
					break;
				}
			}
		}
};

#endif
