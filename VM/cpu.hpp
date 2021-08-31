#ifndef TEA_CPU_HEADER
#define TEA_CPU_HEADER

#include <bits/stdc++.h>

#include "memory.hpp"
#include "../Assembler/executable.hpp"
#include "../Assembler/byte_code.hpp"

using namespace std;

class CPU {
	public:
		// Memory

		uint8_t *ram;

		// Program segment sizes

		size_t static_data_size;
		size_t program_size;
		size_t stack_size;

		// Pointers to common locations in memory

		uint8_t *static_data_location;
		uint8_t *program_location;
		uint8_t *stack_bottom;
		uint8_t *stack_top;

		// Registers

		uint64_t regs[8];

		// General purpose registers

		static constexpr size_t general_purpose_register_count = 4;

		#define R_0 0
		#define R_1 1
		#define R_2 2
		#define R_3 3

		// Special registers

		#define R_INSTR_PTR 4
		void set_instr_ptr(uint8_t *val) { regs[R_INSTR_PTR] = (uint64_t) val; }
		uint8_t *get_instr_ptr() { return (uint8_t *) regs[R_INSTR_PTR]; }

		#define R_STACK_PTR 5
		void set_stack_ptr(uint8_t *val) { regs[R_STACK_PTR] = (uint64_t) val; }
		uint8_t *get_stack_ptr() { return (uint8_t *) regs[R_STACK_PTR]; }

		#define R_FRAME_PTR 6
		void set_frame_ptr(uint8_t *val) { regs[R_FRAME_PTR] = (uint64_t) val; }
		uint8_t *get_frame_ptr() { return (uint8_t *) regs[R_FRAME_PTR]; }

		#define R_RET 7

		uint64_t current_stack_frame_size = 0;

		static const char *reg_to_str(uint8_t reg_id)
		{
			switch (reg_id) {
				default:
					return "UNDEFINED";

				case R_0:
					return "R_0";

				case R_1:
					return "R_1";

				case R_2:
					return "R_2";

				case R_3:
					return "R_3";

				case R_INSTR_PTR:
					return "R_INSTR_PTR";

				case R_STACK_PTR:
					return "R_STACK_PTR";

				case R_FRAME_PTR:
					return "R_FRAME_PTR";

				case R_RET:
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
				program_size(executable.program_size)
		{
			// Initialise the RAM

			size_t ram_size = executable.size + stack_size;
			ram = memory::allocate(ram_size);
			memcpy(ram, executable.data, executable.size);

			// Initialise the common memory locations

			static_data_location = ram;
			program_location     = static_data_location + static_data_size;
			stack_top            = program_location     + program_size;
			stack_bottom         = stack_top            + stack_size;

			// Initialise the registers

			set_instr_ptr(program_location);
			set_stack_ptr(stack_top);
			set_frame_ptr(stack_top);
			regs[R_RET] = 0;
		}

		static constexpr const ssize_t stack_frame_size = 48;

		uint64_t get_reg_by_id(uint8_t id)
		{
			return regs[id];
		}

		void set_reg_by_id(uint8_t id, uint64_t value)
		{
			regs[id] = value;
		}

		Instruction step()
		{
			#ifdef RESTORE_INSTRUCTION_POINTER_ON_THROW
			uint8_t *prev_instr_ptr = get_instr_ptr();
			#endif

			Instruction instruction = (Instruction) fetch<uint16_t>();

			#ifdef RESTORE_INSTRUCTION_POINTER_ON_THROW
			try {
				execute(instruction);
			} catch (const string& err_message) {
				set_instr_ptr(prev_instr_ptr);
				throw err_message;
			}
			#else
			execute(instruction);
			#endif

			return instruction;
		}

		void run()
		{
			while (get_instr_ptr() < stack_top) {
				step();
			}
		}

	private:
		template <typename intx_t>
		intx_t fetch()
		{
			intx_t instruction = memory::get<intx_t>(get_instr_ptr());
			regs[R_INSTR_PTR] += sizeof(intx_t);
			return instruction;
		}

		template <typename intx_t>
		void push(intx_t value)
		{
			memory::set(get_stack_ptr(), value);
			regs[R_STACK_PTR] += sizeof(intx_t);
			current_stack_frame_size += sizeof(intx_t);
		}

		template <typename intx_t>
		intx_t pop()
		{
			regs[R_STACK_PTR] -= sizeof(intx_t);
			intx_t value = memory::get<intx_t>(get_stack_ptr());
			current_stack_frame_size -= sizeof(intx_t);
			return value;
		}

		void push_stack_frame()
		{
			push(get_reg_by_id(R_0));
			push(get_reg_by_id(R_1));
			push(get_reg_by_id(R_2));
			push(get_reg_by_id(R_3));
			push(get_reg_by_id(R_INSTR_PTR));
			push(current_stack_frame_size + 8);

			set_frame_ptr(get_stack_ptr());
			current_stack_frame_size = 0;
		}

		void pop_stack_frame()
		{
			set_stack_ptr(get_frame_ptr());

			current_stack_frame_size = pop<uint64_t>();
			uint64_t saved_stack_frame_size = current_stack_frame_size;

			set_instr_ptr(pop<uint8_t *>());
			regs[R_3] = pop<uint64_t>();
			regs[R_2] = pop<uint64_t>();
			regs[R_1] = pop<uint64_t>();
			regs[R_0] = pop<uint64_t>();

			uint64_t arguments_size = pop<uint64_t>();
			current_stack_frame_size -= arguments_size + 8;

			regs[R_FRAME_PTR] -= saved_stack_frame_size;
			regs[R_STACK_PTR] -= arguments_size;
		}

		void jump_instruction_p(uint8_t *addr)
		{
			set_instr_ptr(program_location + (uint64_t) addr);
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
					uint8_t *address = fetch<uint8_t *>();
					memory::set(address, lit);
					break;
				}

				case MOVE_16_INTO_MEM:
				{
					uint64_t lit = fetch<uint16_t>();
					uint8_t *address = fetch<uint8_t *>();
					memory::set(address, lit);
					break;
				}

				case MOVE_32_INTO_MEM:
				{
					uint64_t lit = fetch<uint32_t>();
					uint8_t *address = fetch<uint8_t *>();
					memory::set(address, lit);
					break;
				}

				case MOVE_64_INTO_MEM:
				{
					uint64_t lit = fetch<uint64_t>();
					uint8_t *address = fetch<uint8_t *>();
					memory::set(address, lit);
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
					uint8_t *address = fetch<uint8_t *>();
					uint8_t value = get_reg_by_id(reg_id) & 0xFF;
					memory::set(address, value);
					break;
				}

				case MOVE_REG_INTO_MEM_16:
				{
					uint8_t reg_id = fetch<uint8_t>();
					uint8_t *address = fetch<uint8_t *>();
					uint16_t value = get_reg_by_id(reg_id) & 0xFFFF;
					memory::set(address, value);
					break;
				}

				case MOVE_REG_INTO_MEM_32:
				{
					uint8_t reg_id = fetch<uint8_t>();
					uint8_t *address = fetch<uint8_t *>();
					uint32_t value = get_reg_by_id(reg_id) & 0xFFFFFFFF;
					memory::set(address, value);
					break;
				}

				case MOVE_REG_INTO_MEM_64:
				{
					uint8_t reg_id = fetch<uint8_t>();
					uint8_t *address = fetch<uint8_t *>();
					uint64_t value = get_reg_by_id(reg_id);
					memory::set(address, value);
					break;
				}

				case MOVE_MEM_8_INTO_REG:
				{
					uint8_t *address = fetch<uint8_t *>();
					uint8_t value = memory::get<uint8_t>(address);
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_MEM_16_INTO_REG:
				{
					uint8_t *address = fetch<uint8_t *>();
					uint16_t value = memory::get<uint16_t>(address);
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_MEM_32_INTO_REG:
				{
					uint8_t *address = fetch<uint8_t *>();
					uint32_t value = memory::get<uint32_t>(address);
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_MEM_64_INTO_REG:
				{
					uint8_t *address = fetch<uint8_t *>();
					uint64_t value = memory::get<uint64_t>(address);
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_REG_POINTER_8_INTO_REG:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					uint8_t *address = (uint8_t *) get_reg_by_id(reg_id_1);
					uint8_t value = memory::get<uint8_t>(address);
					set_reg_by_id(reg_id_2, value);
					break;
				}

				case MOVE_REG_POINTER_16_INTO_REG:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					uint8_t *address = (uint8_t *) get_reg_by_id(reg_id_1);
					uint16_t value = memory::get<uint16_t>(address);
					set_reg_by_id(reg_id_2, value);
					break;
				}

				case MOVE_REG_POINTER_32_INTO_REG:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					uint8_t *address = (uint8_t *) get_reg_by_id(reg_id_1);
					uint32_t value = memory::get<uint32_t>(address);
					set_reg_by_id(reg_id_2, value);
					break;
				}

				case MOVE_REG_POINTER_64_INTO_REG:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					uint8_t *address = (uint8_t *) get_reg_by_id(reg_id_1);
					uint64_t value = memory::get<uint64_t>(address);
					set_reg_by_id(reg_id_2, value);
					break;
				}

				case MOVE_REG_INTO_REG_POINTER_8:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					uint8_t *address = (uint8_t *) get_reg_by_id(reg_id_2);
					uint8_t value = get_reg_by_id(reg_id_1);
					memory::set<uint8_t>(address, value);
					break;
				}

				case MOVE_REG_INTO_REG_POINTER_16:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					uint8_t *address = (uint8_t *) get_reg_by_id(reg_id_2);
					uint16_t value = get_reg_by_id(reg_id_1);
					memory::set<uint16_t>(address, value);
					break;
				}

				case MOVE_REG_INTO_REG_POINTER_32:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					uint8_t *address = (uint8_t *) get_reg_by_id(reg_id_2);
					uint32_t value = get_reg_by_id(reg_id_1);
					memory::set<uint32_t>(address, value);
					break;
				}

				case MOVE_REG_INTO_REG_POINTER_64:
				{
					uint8_t reg_id_1 = fetch<uint8_t>();
					uint8_t reg_id_2 = fetch<uint8_t>();
					uint8_t *address = (uint8_t *) get_reg_by_id(reg_id_2);
					uint64_t value = get_reg_by_id(reg_id_1);
					memory::set<uint64_t>(address, value);
					break;
				}

				case MOVE_FRAME_OFFSET_8_INTO_REG:
				{
					int64_t offset = fetch<int64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					uint8_t value = memory::get<uint8_t>(get_frame_ptr() + offset);
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_FRAME_OFFSET_16_INTO_REG:
				{
					int64_t offset = fetch<int64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					uint16_t value = memory::get<uint16_t>(get_frame_ptr() + offset);
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_FRAME_OFFSET_32_INTO_REG:
				{
					int64_t offset = fetch<int64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					uint32_t value = memory::get<uint32_t>(get_frame_ptr() + offset);
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_FRAME_OFFSET_64_INTO_REG:
				{
					int64_t offset = fetch<int64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					uint64_t value = memory::get<uint64_t>(get_frame_ptr() + offset);
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_REG_INTO_FRAME_OFFSET_8:
				{
					uint8_t reg_id = fetch<uint8_t>();
					int64_t offset = fetch<uint64_t>();
					uint8_t value = get_reg_by_id(reg_id) & 0xFF;
					memory::set(get_frame_ptr() + offset, value);
					break;
				}

				case MOVE_REG_INTO_FRAME_OFFSET_16:
				{
					uint8_t reg_id = fetch<uint8_t>();
					int64_t offset = fetch<uint64_t>();
					uint16_t value = get_reg_by_id(reg_id) & 0xFFFF;
					memory::set(get_frame_ptr() + offset, value);
					break;
				}

				case MOVE_REG_INTO_FRAME_OFFSET_32:
				{
					uint8_t reg_id = fetch<uint8_t>();
					int64_t offset = fetch<uint64_t>();
					uint32_t value = get_reg_by_id(reg_id) & 0xFFFFFFFF;
					memory::set(get_frame_ptr() + offset, value);
					break;
				}

				case MOVE_REG_INTO_FRAME_OFFSET_64:
				{
					uint8_t reg_id = fetch<uint8_t>();
					int64_t offset = fetch<uint64_t>();
					uint64_t value = get_reg_by_id(reg_id);
					memory::set(get_frame_ptr() + offset, value);
					break;
				}

				case MOVE_STACK_TOP_OFFSET_8_INTO_REG:
				{
					int64_t offset = fetch<int64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					uint8_t value = memory::get<uint8_t>(stack_top + offset);
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_STACK_TOP_OFFSET_16_INTO_REG:
				{
					int64_t offset = fetch<int64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					uint16_t value = memory::get<uint16_t>(stack_top + offset);
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_STACK_TOP_OFFSET_32_INTO_REG:
				{
					int64_t offset = fetch<int64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					uint32_t value = memory::get<uint32_t>(stack_top + offset);
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_STACK_TOP_OFFSET_64_INTO_REG:
				{
					int64_t offset = fetch<int64_t>();
					uint8_t reg_id = fetch<uint8_t>();
					uint64_t value = memory::get<uint64_t>(stack_top + offset);
					set_reg_by_id(reg_id, value);
					break;
				}

				case MOVE_REG_INTO_STACK_TOP_OFFSET_8:
				{
					uint8_t reg_id = fetch<uint8_t>();
					int64_t offset = fetch<int64_t>();
					uint8_t value = get_reg_by_id(reg_id) & 0xFF;
					memory::set(stack_top + offset, value);
					break;
				}

				case MOVE_REG_INTO_STACK_TOP_OFFSET_16:
				{
					uint8_t reg_id = fetch<uint8_t>();
					int64_t offset = fetch<int64_t>();
					uint16_t value = get_reg_by_id(reg_id) & 0xFFFF;
					memory::set(stack_top + offset, value);
					break;
				}

				case MOVE_REG_INTO_STACK_TOP_OFFSET_32:
				{
					uint8_t reg_id = fetch<uint8_t>();
					int64_t offset = fetch<int64_t>();
					uint32_t value = get_reg_by_id(reg_id) & 0xFFFFFFFF;
					memory::set(stack_top + offset, value);
					break;
				}

				case MOVE_REG_INTO_STACK_TOP_OFFSET_64:
				{
					uint8_t reg_id = fetch<uint8_t>();
					int64_t offset = fetch<int64_t>();
					uint64_t value = get_reg_by_id(reg_id);
					memory::set(stack_top + offset, value);
					break;
				}

				case MOVE_STACK_TOP_ADDRESS_INTO_REG:
				{
					uint8_t reg_id = fetch<uint8_t>();
					set_reg_by_id(reg_id, (uint64_t) stack_top);
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
					uint8_t *address = fetch<uint8_t *>();
					jump_instruction_p(address);
					break;
				}

				case JUMP_IF_GREATER:
				{
					uint8_t *address = fetch<uint8_t *>();
					if (greater_flag) jump_instruction_p(address);
					break;
				}

				case JUMP_IF_GREATER_OR_EQUAL:
				{
					uint8_t *address = fetch<uint8_t *>();
					if (greater_flag | equal_flag) jump_instruction_p(address);
					break;
				}

				case JUMP_IF_LESS:
				{
					uint8_t *address = fetch<uint8_t *>();
					if (!greater_flag & !equal_flag) jump_instruction_p(address);
					break;
				}

				case JUMP_IF_LESS_OR_EQUAL:
				{
					uint8_t *address = fetch<uint8_t *>();
					if (!greater_flag) jump_instruction_p(address);
					break;
				}

				case JUMP_IF_EQUAL:
				{
					uint8_t *address = fetch<uint8_t *>();
					if (equal_flag) jump_instruction_p(address);
					break;
				}

				case JUMP_IF_NOT_EQUAL:
				{
					uint8_t *address = fetch<uint8_t *>();
					if (!equal_flag) jump_instruction_p(address);
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
					uint8_t *address = fetch<uint8_t *>();
					push_stack_frame();
					jump_instruction_p(address);
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
					regs[R_STACK_PTR] += size;
					current_stack_frame_size += size;
					break;
				}

				case DEALLOCATE_STACK:
				{
					uint64_t size = fetch<uint64_t>();
					regs[R_STACK_PTR] -= size;
					current_stack_frame_size -= size;
					break;
				}

				case COMMENT:
				case LABEL:
				{
					while (fetch<uint8_t>() != '\0');
					break;
				}

				case PRINT_CHAR:
				{
					uint8_t reg_id = fetch<uint8_t>();
					uint64_t value = get_reg_by_id(reg_id);
					putc(value, stdout);
					break;
				}

				case GET_CHAR:
				{
					uint8_t reg_id = fetch<uint8_t>();
					int16_t c = getc(stdin);
					set_reg_by_id(reg_id, c);
					break;
				}
			}
		}
};

#endif
