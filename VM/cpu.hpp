#ifndef TEA_CPU_HEADER
#define TEA_CPU_HEADER

#include "VM/memory.hpp"
#include "Executable/executable.hpp"
#include "Executable/byte-code.hpp"

/**
 * @brief The class that represents a CPU of the virtual machine.
 * Contains methods to run an executable.
 */
struct CPU
{
	// ===== Program segment sizes =====

	// The size of the static data segment in bytes.
	size_t static_data_size;

	// The size of the program size in bytes.
	size_t program_size;

	// The size of the stack in bytes.
	size_t stack_size;

	// ===== Pointers to common locations in memory =====

	// A pointer to the start of the static data segment.
	uint8_t *static_data_location;

	// A pointer to the start of the program segment.
	uint8_t *program_location;

	// A pointer to the bottom (start) of the stack.
	uint8_t *stack_bottom;

	// A pointer to the top (end) of the stack.
	uint8_t *stack_top;

	// The number of general purpose registers (R_0, R_1, ...)
#define GENERAL_PURPOSE_REGISTER_COUNT 16
#define TOTAL_REGISTER_COUNT           GENERAL_PURPOSE_REGISTER_COUNT + 5

	// ===== Registers =====

	// An array that contains the registers of the virtual machine.
	uint64_t regs[TOTAL_REGISTER_COUNT];

	// === General purpose registers ===

#define R_0  0
#define R_1  1
#define R_2  2
#define R_3  3
#define R_4  4
#define R_5  5
#define R_6  6
#define R_7  7
#define R_8  8
#define R_9  9
#define R_10 10
#define R_11 11
#define R_12 12
#define R_13 13
#define R_14 14
#define R_15 15

	// === Special registers ===

	// Instruction register

#define R_INSTR_PTR GENERAL_PURPOSE_REGISTER_COUNT + 0

	/**
	 * @brief Sets the instruction pointer to a certain value.
	 * @param val A memory address to store in the IR register.
	 */
	void
	set_instr_ptr(uint8_t *val)
	{
		regs[R_INSTR_PTR] = (uint64_t) val;
	}

	/**
	 * @returns The memory address in the IR register.
	 */
	uint8_t *
	get_instr_ptr()
	{
		return (uint8_t *) regs[R_INSTR_PTR];
	}

	// Stack pointer

#define R_STACK_PTR GENERAL_PURPOSE_REGISTER_COUNT + 1

	/**
	 * @brief Set the stack pointer to a certain value.
	 * @param val A memory address to store in the SP register.
	 */
	void
	set_stack_ptr(uint8_t *val)
	{
		regs[R_STACK_PTR] = (uint64_t) val;
	}

	/**
	 * @returns The memory address in the SP register.
	 */
	uint8_t *
	get_stack_ptr()
	{
		return (uint8_t *) regs[R_STACK_PTR];
	}

	// Stack top pointer

#define R_STACK_TOP_PTR GENERAL_PURPOSE_REGISTER_COUNT + 2

	/**
	 * @brief Sets the stack top pointer to a certain value.
	 * @param val A memory address to store in the ST register.
	 */
	void
	set_stack_top_ptr(uint8_t *val)
	{
		regs[R_STACK_TOP_PTR] = (uint64_t) val;
	}

	/**
	 * @returns The memory address in the ST register.
	 */
	uint8_t *
	get_stack_top_ptr()
	{
		return (uint8_t *) regs[R_STACK_TOP_PTR];
	}

	// Frame pointer

#define R_FRAME_PTR GENERAL_PURPOSE_REGISTER_COUNT + 3

	/**
	 * @brief Sets the frame pointer to a certain value.
	 * @param val A memory address the store in the FP register.
	 */
	void
	set_frame_ptr(uint8_t *val)
	{
		regs[R_FRAME_PTR] = (uint64_t) val;
	}

	/**
	 * @returns The memory address in the FP register.
	 */
	uint8_t *
	get_frame_ptr()
	{
		return (uint8_t *) regs[R_FRAME_PTR];
	}

	// Return value register

#define R_RET GENERAL_PURPOSE_REGISTER_COUNT + 4

// The size of a stack frame. This consists of the old values of the
// general purpose registers, the old instruction pointer
// (used as return address), and the size parameters in bytes.
#define STACK_FRAME_SIZE (GENERAL_PURPOSE_REGISTER_COUNT + 5) * 8

	// Holds the address of the current instruction being executed.
	uint8_t *cur_instr_addr;

	// Converts a register id to a register name.
	static const char *
	reg_to_str(uint8_t reg_id)
	{
		switch (reg_id)
		{
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
		case R_4:
			return "R_4";
		case R_5:
			return "R_5";
		case R_6:
			return "R_6";
		case R_7:
			return "R_7";
		case R_8:
			return "R_8";
		case R_9:
			return "R_9";
		case R_10:
			return "R_10";
		case R_11:
			return "R_11";
		case R_12:
			return "R_12";
		case R_13:
			return "R_13";
		case R_14:
			return "R_14";
		case R_15:
			return "R_15";

		case R_INSTR_PTR:
			return "R_INSTR_PTR";

		case R_STACK_PTR:
			return "R_STACK_PTR";

		case R_STACK_TOP_PTR:
			return "R_STACK_TOP_PTR";

		case R_FRAME_PTR:
			return "R_FRAME_PTR";

		case R_RET:
			return "R_RET";
		}
	}

	// Flags

	bool overflow_flag       = false;
	bool division_error_flag = false;
	bool equal_flag          = false;
	bool greater_flag        = false;

	/**
	 * @brief Constructs a new CPU object.
	 * Creates RAM and initialises locations and registers.
	 * @param executable A reference to the executable to run.
	 * @param stack_size The stack size of the virtual machine.
	 */
	CPU(Executable &executable, size_t stack_size)
		: static_data_size(executable.static_data_size),
		  program_size(executable.program_size),
		  stack_size(stack_size)
	{
		// Initialise the memory regions

		// 1. Program region, contains the executable code.
		uint8_t *program_region = memory::allocate(program_size);
		memcpy(program_region, executable.data + static_data_size, program_size);

		// 2. Stack region, contains the stack, prepended by the static data.
		uint8_t *stack_region = memory::allocate(static_data_size + stack_size);
		memcpy(stack_region, executable.data, static_data_size);

		// Initialise the common memory locations

		program_location     = program_region;
		static_data_location = stack_region;
		stack_top            = stack_region + static_data_size;
		stack_bottom         = stack_top + stack_size;

		// Initialise the registers

		set_instr_ptr(program_location);
		set_stack_ptr(stack_top);
		set_frame_ptr(stack_top);
		set_stack_top_ptr(stack_top);

		// If the main function of a program does not return
		// anything, returns 0 by default to indicate
		// successful termination.

		regs[R_RET] = 0;
	}

	/**
	 * @param id The id of the register to get.
	 * @returns The value inside the register.
	 */
	uint64_t
	get_reg_by_id(uint8_t id)
	{
		return regs[id];
	}

	/**
	 * @param id The id of the register to set.
	 * @param value The value to store inside the register.
	 */
	void
	set_reg_by_id(uint8_t id, uint64_t value)
	{
		regs[id] = value;
	}

	/**
	 * @brief Executes the next instruction of the executable.
	 * @returns The opcode of the executed instruction.
	 */
	Instruction
	step()
	{
		cur_instr_addr = get_instr_ptr();

		Instruction instruction = (Instruction) fetch<uint16_t>();

#ifdef RESTORE_INSTRUCTION_POINTER_ON_THROW
		try
		{
			execute(instruction);
		}
		catch (const std::string &err_message)
		{
			set_instr_ptr(cur_instr_addr);
			throw err_message;
		}
#else
		execute(instruction);
#endif

		return instruction;
	}

	/**
	 * @brief Runs the executable until it crashes or completes.
	 */
	void
	run()
	{
		while (get_instr_ptr() < program_location + program_size)
		{
			step();
		}
	}

	/**
	 * @brief Fetches a value from the current instruction pointer.
	 * Increments the instruction pointer by the size of the read.
	 * @tparam intx_t The size of the value to fetch.
	 * @returns The value at the current instruction pointer.
	 */
	template <typename intx_t>
	intx_t
	fetch()
	{
		intx_t instruction = memory::get<intx_t>(get_instr_ptr());
		regs[R_INSTR_PTR] += sizeof(intx_t);
		return instruction;
	}

	/**
	 * @brief Pushes a value onto the stack.
	 * @tparam intx_t The size of the value to push.
	 * @param value The value to push.
	 */
	template <typename intx_t>
	void
	push(intx_t value)
	{
		memory::set(get_stack_ptr(), value);
		regs[R_STACK_PTR] += sizeof(intx_t);
	}

	/**
	 * @brief Pops a value off the stack.
	 * @tparam intx_t The size of the value to pop.
	 * @returns The value popped off the stack.
	 */
	template <typename intx_t>
	intx_t
	pop()
	{
		regs[R_STACK_PTR] -= sizeof(intx_t);
		intx_t value = memory::get<intx_t>(get_stack_ptr());
		return value;
	}

	/**
	 * @brief Pushes a stack frame.
	 * Executed when a function is called.
	 * A stack frame consists of the old values of the four
	 * general purpose registers,
	 * the old instruction pointer (used as return address),
	 * and the size parameters in bytes.
	 */
	void
	push_stack_frame()
	{
		push(get_reg_by_id(R_0));
		push(get_reg_by_id(R_1));
		push(get_reg_by_id(R_2));
		push(get_reg_by_id(R_3));
		push(get_reg_by_id(R_4));
		push(get_reg_by_id(R_5));
		push(get_reg_by_id(R_6));
		push(get_reg_by_id(R_7));
		push(get_reg_by_id(R_8));
		push(get_reg_by_id(R_9));
		push(get_reg_by_id(R_10));
		push(get_reg_by_id(R_11));
		push(get_reg_by_id(R_12));
		push(get_reg_by_id(R_13));
		push(get_reg_by_id(R_14));
		push(get_reg_by_id(R_15));
		push(get_instr_ptr());
		push(get_frame_ptr());

		set_frame_ptr(get_stack_ptr());
	}

	/**
	 * @brief Pops a stack frame.
	 * Executed when a function returns.
	 * A stack frame consists of the old values of the four
	 * general purpose registers,
	 * the old instruction pointer (used as return address),
	 * and the size parameters in bytes.
	 */
	void
	pop_stack_frame()
	{
		set_stack_ptr(get_frame_ptr());

		set_frame_ptr(pop<uint8_t *>());
		set_instr_ptr(pop<uint8_t *>());
		set_reg_by_id(R_15, pop<uint64_t>());
		set_reg_by_id(R_14, pop<uint64_t>());
		set_reg_by_id(R_13, pop<uint64_t>());
		set_reg_by_id(R_12, pop<uint64_t>());
		set_reg_by_id(R_11, pop<uint64_t>());
		set_reg_by_id(R_10, pop<uint64_t>());
		set_reg_by_id(R_9, pop<uint64_t>());
		set_reg_by_id(R_8, pop<uint64_t>());
		set_reg_by_id(R_7, pop<uint64_t>());
		set_reg_by_id(R_6, pop<uint64_t>());
		set_reg_by_id(R_5, pop<uint64_t>());
		set_reg_by_id(R_4, pop<uint64_t>());
		set_reg_by_id(R_3, pop<uint64_t>());
		set_reg_by_id(R_2, pop<uint64_t>());
		set_reg_by_id(R_1, pop<uint64_t>());
		set_reg_by_id(R_0, pop<uint64_t>());

		regs[R_STACK_PTR] -= pop<uint64_t>(); // Args size
	}

	/**
	 * @brief Sets the instruction pointer to an offset from
	 * the current instruction pointer.
	 * @param offset The relative offset.
	 */
	void
	jump_instruction_p(int64_t offset)
	{
		set_instr_ptr(cur_instr_addr + offset);
	}

	/**
	 * @brief Executes an instruction.
	 * @param instruction The instruction to execute.
	 */
	void
	execute(uint16_t instruction)
	{
		// Giant switch statement for all instructions.
		// If you're unsure about what exactly an instruction
		// is supposed to do, check out the descriptions
		// in the `Instruction` enum.
		// There are no comments in this switch statement,
		// because the descriptions are self-explanatory.
		// There is no code after the switch statement,
		// so don't bother scrolling down.

		switch (instruction)
		{
		case MOVE_LIT:
		{
			uint64_t lit   = fetch<uint64_t>();
			uint8_t reg_id = fetch<uint8_t>();
			set_reg_by_id(reg_id, lit);
			break;
		}

		case MOVE:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			uint64_t value   = get_reg_by_id(reg_id_1);
			set_reg_by_id(reg_id_2, value);
			break;
		}

		case LOAD_PTR_8:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			uint8_t *address = (uint8_t *) get_reg_by_id(reg_id_1);
			uint8_t value    = memory::get<uint8_t>(address);
			set_reg_by_id(reg_id_2, value);
			break;
		}

		case LOAD_PTR_16:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			uint8_t *address = (uint8_t *) get_reg_by_id(reg_id_1);
			uint16_t value   = memory::get<uint16_t>(address);
			set_reg_by_id(reg_id_2, value);
			break;
		}

		case LOAD_PTR_32:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			uint8_t *address = (uint8_t *) get_reg_by_id(reg_id_1);
			uint32_t value   = memory::get<uint32_t>(address);
			set_reg_by_id(reg_id_2, value);
			break;
		}

		case LOAD_PTR_64:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			uint8_t *address = (uint8_t *) get_reg_by_id(reg_id_1);
			uint64_t value   = memory::get<uint64_t>(address);
			set_reg_by_id(reg_id_2, value);
			break;
		}

		case STORE_PTR_8:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			uint8_t *address = (uint8_t *) get_reg_by_id(reg_id_2);
			uint8_t value    = get_reg_by_id(reg_id_1);
			memory::set<uint8_t>(address, value);
			break;
		}

		case STORE_PTR_16:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			uint8_t *address = (uint8_t *) get_reg_by_id(reg_id_2);
			uint16_t value   = get_reg_by_id(reg_id_1);
			memory::set<uint16_t>(address, value);
			break;
		}

		case STORE_PTR_32:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			uint8_t *address = (uint8_t *) get_reg_by_id(reg_id_2);
			uint32_t value   = get_reg_by_id(reg_id_1);
			memory::set<uint32_t>(address, value);
			break;
		}

		case STORE_PTR_64:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			uint8_t *address = (uint8_t *) get_reg_by_id(reg_id_2);
			uint64_t value   = get_reg_by_id(reg_id_1);
			memory::set<uint64_t>(address, value);
			break;
		}

		case MEM_COPY:
		{
			uint8_t reg_id_src = fetch<uint8_t>();
			uint8_t reg_id_dst = fetch<uint8_t>();
			uint64_t n_bytes   = fetch<uint64_t>();

			uint8_t *src_address = (uint8_t *) get_reg_by_id(reg_id_src);
			uint8_t *dst_address = (uint8_t *) get_reg_by_id(reg_id_dst);

			for (uint64_t i = 0; i < n_bytes; i++)
			{
				uint8_t value = memory::get<uint8_t>(src_address + i);
				memory::set(dst_address + i, value);
			}

			break;
		}

		case ADD_INT_8:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			set_reg_by_id(reg_id_2,
				static_cast<uint8_t>(get_reg_by_id(reg_id_1))
					+ static_cast<uint8_t>(get_reg_by_id(reg_id_2)));
			break;
		}

		case ADD_INT_16:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			set_reg_by_id(reg_id_2,
				static_cast<uint16_t>(get_reg_by_id(reg_id_1))
					+ static_cast<uint16_t>(get_reg_by_id(reg_id_2)));
			break;
		}

		case ADD_INT_32:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			set_reg_by_id(reg_id_2,
				static_cast<uint32_t>(get_reg_by_id(reg_id_1))
					+ static_cast<uint32_t>(get_reg_by_id(reg_id_2)));
			break;
		}

		case ADD_INT_64:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			set_reg_by_id(reg_id_2,
				static_cast<uint64_t>(get_reg_by_id(reg_id_1))
					+ static_cast<uint64_t>(get_reg_by_id(reg_id_2)));
			break;
		}

		case ADD_FLT_32:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			uint32_t value_1 = static_cast<uint32_t>(get_reg_by_id(reg_id_1));
			uint32_t value_2 = static_cast<uint32_t>(get_reg_by_id(reg_id_2));
			set_reg_by_id(reg_id_2,
				*reinterpret_cast<float *>(&value_1)
					+ *reinterpret_cast<float *>(&value_2));
			break;
		}

		case ADD_FLT_64:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			uint64_t value_1 = get_reg_by_id(reg_id_1);
			uint64_t value_2 = get_reg_by_id(reg_id_2);
			set_reg_by_id(reg_id_2,
				*reinterpret_cast<double *>(&value_1)
					+ *reinterpret_cast<double *>(&value_2));
			break;
		}

		case SUB_INT_8:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			set_reg_by_id(reg_id_2,
				static_cast<uint8_t>(get_reg_by_id(reg_id_2))
					- static_cast<uint8_t>(get_reg_by_id(reg_id_1)));
			break;
		}

		case SUB_INT_16:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			set_reg_by_id(reg_id_2,
				static_cast<uint16_t>(get_reg_by_id(reg_id_2))
					- static_cast<uint16_t>(get_reg_by_id(reg_id_1)));
			break;
		}

		case SUB_INT_32:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			set_reg_by_id(reg_id_2,
				static_cast<uint32_t>(get_reg_by_id(reg_id_2))
					- static_cast<uint32_t>(get_reg_by_id(reg_id_1)));
			break;
		}

		case SUB_INT_64:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			set_reg_by_id(reg_id_2,
				static_cast<uint64_t>(get_reg_by_id(reg_id_2))
					- static_cast<uint64_t>(get_reg_by_id(reg_id_1)));
			break;
		}

		case SUB_FLT_32:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			uint32_t value_1 = static_cast<uint32_t>(get_reg_by_id(reg_id_1));
			uint32_t value_2 = static_cast<uint32_t>(get_reg_by_id(reg_id_2));
			set_reg_by_id(reg_id_2,
				*reinterpret_cast<float *>(&value_2)
					- *reinterpret_cast<float *>(&value_1));
			break;
		}

		case SUB_FLT_64:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			uint64_t value_1 = get_reg_by_id(reg_id_1);
			uint64_t value_2 = get_reg_by_id(reg_id_2);
			set_reg_by_id(reg_id_2,
				*reinterpret_cast<double *>(&value_2)
					- *reinterpret_cast<double *>(&value_1));
			break;
		}

		case MUL_INT_8:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			set_reg_by_id(reg_id_2,
				static_cast<uint8_t>(get_reg_by_id(reg_id_1))
					* static_cast<uint8_t>(get_reg_by_id(reg_id_2)));
			break;
		}

		case MUL_INT_16:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			set_reg_by_id(reg_id_2,
				static_cast<uint16_t>(get_reg_by_id(reg_id_1))
					* static_cast<uint16_t>(get_reg_by_id(reg_id_2)));
			break;
		}

		case MUL_INT_32:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			set_reg_by_id(reg_id_2,
				static_cast<uint32_t>(get_reg_by_id(reg_id_1))
					* static_cast<uint32_t>(get_reg_by_id(reg_id_2)));
			break;
		}

		case MUL_INT_64:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			set_reg_by_id(reg_id_2,
				static_cast<uint64_t>(get_reg_by_id(reg_id_1))
					* static_cast<uint64_t>(get_reg_by_id(reg_id_2)));
			break;
		}

		case MUL_FLT_32:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			uint32_t value_1 = static_cast<uint32_t>(get_reg_by_id(reg_id_1));
			uint32_t value_2 = static_cast<uint32_t>(get_reg_by_id(reg_id_2));
			set_reg_by_id(reg_id_2,
				*reinterpret_cast<float *>(&value_1)
					* *reinterpret_cast<float *>(&value_2));
			break;
		}

		case MUL_FLT_64:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			uint64_t value_1 = get_reg_by_id(reg_id_1);
			uint64_t value_2 = get_reg_by_id(reg_id_2);
			set_reg_by_id(reg_id_2,
				*reinterpret_cast<double *>(&value_1)
					* *reinterpret_cast<double *>(&value_2));
			break;
		}

		case DIV_INT_8:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();

			uint8_t value_1 = static_cast<uint8_t>(get_reg_by_id(reg_id_1));
			uint8_t value_2 = static_cast<uint8_t>(get_reg_by_id(reg_id_2));

			if (value_1 == 0)
			{
				division_error_flag = true;
				break;
			}

			set_reg_by_id(reg_id_2, value_2 / value_1);
			break;
		}

		case DIV_INT_16:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();

			uint16_t value_1 = static_cast<uint16_t>(get_reg_by_id(reg_id_1));
			uint16_t value_2 = static_cast<uint16_t>(get_reg_by_id(reg_id_2));

			if (value_1 == 0)
			{
				division_error_flag = true;
				break;
			}

			set_reg_by_id(reg_id_2, value_2 / value_1);
			break;
		}

		case DIV_INT_32:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();

			uint32_t value_1 = static_cast<uint32_t>(get_reg_by_id(reg_id_1));
			uint32_t value_2 = static_cast<uint32_t>(get_reg_by_id(reg_id_2));

			if (value_1 == 0)
			{
				division_error_flag = true;
				break;
			}

			set_reg_by_id(reg_id_2, value_2 / value_1);
			break;
		}

		case DIV_INT_64:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();

			uint64_t value_1 = get_reg_by_id(reg_id_1);
			uint64_t value_2 = get_reg_by_id(reg_id_2);

			if (value_1 == 0)
			{
				division_error_flag = true;
				break;
			}

			set_reg_by_id(reg_id_2, value_2 / value_1);
			break;
		}

		case DIV_FLT_32:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();

			uint32_t value_1 = static_cast<uint32_t>(get_reg_by_id(reg_id_1));
			uint32_t value_2 = static_cast<uint32_t>(get_reg_by_id(reg_id_2));

			if (value_1 == 0)
			{
				division_error_flag = true;
				break;
			}

			set_reg_by_id(reg_id_2,
				*reinterpret_cast<float *>(&value_2)
					/ *reinterpret_cast<float *>(&value_1));
			break;
		}

		case DIV_FLT_64:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();

			uint64_t value_1 = get_reg_by_id(reg_id_1);
			uint64_t value_2 = get_reg_by_id(reg_id_2);

			if (value_1 == 0)
			{
				division_error_flag = true;
				break;
			}

			set_reg_by_id(reg_id_2,
				*reinterpret_cast<double *>(&value_2)
					/ *reinterpret_cast<double *>(&value_1));
			break;
		}

		case MOD_INT_8:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();

			uint8_t value_1 = static_cast<uint8_t>(get_reg_by_id(reg_id_1));
			uint8_t value_2 = static_cast<uint8_t>(get_reg_by_id(reg_id_2));

			if (value_1 == 0)
			{
				division_error_flag = true;
				break;
			}

			set_reg_by_id(reg_id_2, value_2 % value_1);
			break;
		}

		case MOD_INT_16:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();

			uint16_t value_1 = static_cast<uint16_t>(get_reg_by_id(reg_id_1));
			uint16_t value_2 = static_cast<uint16_t>(get_reg_by_id(reg_id_2));

			if (value_1 == 0)
			{
				division_error_flag = true;
				break;
			}

			set_reg_by_id(reg_id_2, value_2 % value_1);
			break;
		}

		case MOD_INT_32:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();

			uint32_t value_1 = static_cast<uint32_t>(get_reg_by_id(reg_id_1));
			uint32_t value_2 = static_cast<uint32_t>(get_reg_by_id(reg_id_2));

			if (value_1 == 0)
			{
				division_error_flag = true;
				break;
			}

			set_reg_by_id(reg_id_2, value_2 % value_1);
			break;
		}

		case MOD_INT_64:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();

			uint64_t value_1 = get_reg_by_id(reg_id_1);
			uint64_t value_2 = get_reg_by_id(reg_id_2);

			if (value_1 == 0)
			{
				division_error_flag = true;
				break;
			}

			set_reg_by_id(reg_id_2, value_2 % value_1);
			break;
		}

		case AND_INT_8:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			set_reg_by_id(reg_id_2, get_reg_by_id(reg_id_2) & get_reg_by_id(reg_id_1));
			break;
		}

		case AND_INT_16:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			set_reg_by_id(reg_id_2, get_reg_by_id(reg_id_2) & get_reg_by_id(reg_id_1));
			break;
		}

		case AND_INT_32:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			set_reg_by_id(reg_id_2, get_reg_by_id(reg_id_2) & get_reg_by_id(reg_id_1));
			break;
		}

		case AND_INT_64:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			set_reg_by_id(reg_id_2, get_reg_by_id(reg_id_2) & get_reg_by_id(reg_id_1));
			break;
		}

		case OR_INT_8:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			set_reg_by_id(reg_id_2, get_reg_by_id(reg_id_2) | get_reg_by_id(reg_id_1));
			break;
		}

		case OR_INT_16:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			set_reg_by_id(reg_id_2, get_reg_by_id(reg_id_2) | get_reg_by_id(reg_id_1));
			break;
		}

		case OR_INT_32:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			set_reg_by_id(reg_id_2, get_reg_by_id(reg_id_2) | get_reg_by_id(reg_id_1));
			break;
		}

		case OR_INT_64:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			set_reg_by_id(reg_id_2, get_reg_by_id(reg_id_2) | get_reg_by_id(reg_id_1));
			break;
		}

		case XOR_INT_8:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			set_reg_by_id(reg_id_2, get_reg_by_id(reg_id_2) ^ get_reg_by_id(reg_id_1));
			break;
		}

		case XOR_INT_16:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			set_reg_by_id(reg_id_2, get_reg_by_id(reg_id_2) ^ get_reg_by_id(reg_id_1));
			break;
		}

		case XOR_INT_32:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			set_reg_by_id(reg_id_2, get_reg_by_id(reg_id_2) ^ get_reg_by_id(reg_id_1));
			break;
		}

		case XOR_INT_64:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			set_reg_by_id(reg_id_2, get_reg_by_id(reg_id_2) ^ get_reg_by_id(reg_id_1));
			break;
		}

		case SHL_INT_8:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			uint8_t value    = static_cast<uint8_t>(get_reg_by_id(reg_id_2));
			uint8_t shift    = static_cast<uint8_t>(get_reg_by_id(reg_id_1));
			set_reg_by_id(reg_id_2, value << shift);
			break;
		}

		case SHL_INT_16:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			uint16_t value   = static_cast<uint16_t>(get_reg_by_id(reg_id_2));
			uint16_t shift   = static_cast<uint16_t>(get_reg_by_id(reg_id_1));
			set_reg_by_id(reg_id_2, value << shift);
			break;
		}

		case SHL_INT_32:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			uint32_t value   = static_cast<uint32_t>(get_reg_by_id(reg_id_2));
			uint32_t shift   = static_cast<uint32_t>(get_reg_by_id(reg_id_1));
			set_reg_by_id(reg_id_2, value << shift);
			break;
		}

		case SHL_INT_64:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			uint64_t value   = static_cast<uint64_t>(get_reg_by_id(reg_id_2));
			uint64_t shift   = static_cast<uint64_t>(get_reg_by_id(reg_id_1));
			set_reg_by_id(reg_id_2, value << shift);
			break;
		}

		case SHR_INT_8:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			uint8_t value    = static_cast<uint8_t>(get_reg_by_id(reg_id_2));
			uint8_t shift    = static_cast<uint8_t>(get_reg_by_id(reg_id_1));
			set_reg_by_id(reg_id_2, value >> shift);
			break;
		}

		case SHR_INT_16:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			uint16_t value   = static_cast<uint16_t>(get_reg_by_id(reg_id_2));
			uint16_t shift   = static_cast<uint16_t>(get_reg_by_id(reg_id_1));
			set_reg_by_id(reg_id_2, value >> shift);
			break;
		}

		case SHR_INT_32:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			uint32_t value   = static_cast<uint32_t>(get_reg_by_id(reg_id_2));
			uint32_t shift   = static_cast<uint32_t>(get_reg_by_id(reg_id_1));
			set_reg_by_id(reg_id_2, value >> shift);
			break;
		}

		case SHR_INT_64:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();
			uint64_t value   = static_cast<uint64_t>(get_reg_by_id(reg_id_2));
			uint64_t shift   = static_cast<uint64_t>(get_reg_by_id(reg_id_1));
			set_reg_by_id(reg_id_2, value >> shift);
			break;
		}

		case INC_INT_8:
		{
			uint8_t reg_id = fetch<uint8_t>();
			uint8_t value  = get_reg_by_id(reg_id);
			value++;
			set_reg_by_id(reg_id, value);
			break;
		}

		case INC_INT_16:
		{
			uint8_t reg_id = fetch<uint8_t>();
			uint16_t value = get_reg_by_id(reg_id);
			value++;
			set_reg_by_id(reg_id, value);
			break;
		}

		case INC_INT_32:
		{
			uint8_t reg_id = fetch<uint8_t>();
			uint32_t value = get_reg_by_id(reg_id);
			value++;
			set_reg_by_id(reg_id, value);
			break;
		}

		case INC_INT_64:
		{
			uint8_t reg_id = fetch<uint8_t>();
			uint64_t value = get_reg_by_id(reg_id);
			value++;
			set_reg_by_id(reg_id, value);
			break;
		}

		case DEC_INT_8:
		{
			uint8_t reg_id = fetch<uint8_t>();
			uint8_t value  = get_reg_by_id(reg_id);
			value--;
			set_reg_by_id(reg_id, value);
			break;
		}

		case DEC_INT_16:
		{
			uint8_t reg_id = fetch<uint8_t>();
			uint8_t value  = get_reg_by_id(reg_id);
			value--;
			set_reg_by_id(reg_id, value);
			break;
		}

		case DEC_INT_32:
		{
			uint8_t reg_id = fetch<uint8_t>();
			uint8_t value  = get_reg_by_id(reg_id);
			value--;
			set_reg_by_id(reg_id, value);
			break;
		}

		case DEC_INT_64:
		{
			uint8_t reg_id = fetch<uint8_t>();
			uint8_t value  = get_reg_by_id(reg_id);
			value--;
			set_reg_by_id(reg_id, value);
			break;
		}

		case NEG_INT_8:
		{
			uint8_t reg_id = fetch<uint8_t>();
			set_reg_by_id(reg_id, -static_cast<int8_t>(get_reg_by_id(reg_id)));
			break;
		}

		case NEG_INT_16:
		{
			uint8_t reg_id = fetch<uint8_t>();
			set_reg_by_id(reg_id, -static_cast<int16_t>(get_reg_by_id(reg_id)));
			break;
		}

		case NEG_INT_32:
		{
			uint8_t reg_id = fetch<uint8_t>();
			set_reg_by_id(reg_id, -static_cast<int32_t>(get_reg_by_id(reg_id)));
			break;
		}

		case NEG_INT_64:
		{
			uint8_t reg_id = fetch<uint8_t>();
			set_reg_by_id(reg_id, -static_cast<int64_t>(get_reg_by_id(reg_id)));
			break;
		}

		case CAST_INT_TO_FLT_32:
		{
			uint8_t reg_id = fetch<uint8_t>();
			float value    = static_cast<float>(get_reg_by_id(reg_id));
			set_reg_by_id(reg_id, *reinterpret_cast<int32_t *>(&value));
			break;
		}

		case CAST_INT_TO_FLT_64:
		{
			uint8_t reg_id = fetch<uint8_t>();
			double value   = static_cast<double>(get_reg_by_id(reg_id));
			set_reg_by_id(reg_id, *reinterpret_cast<int64_t *>(&value));
			break;
		}

		case CAST_FLT_32_TO_INT:
		{
			uint8_t reg_id = fetch<uint8_t>();
			uint32_t value = get_reg_by_id(reg_id);
			float f_value  = *reinterpret_cast<float *>(&value);
			set_reg_by_id(reg_id, static_cast<int64_t>(f_value));
			break;
		}

		case CAST_FLT_64_TO_INT:
		{
			uint8_t reg_id = fetch<uint8_t>();
			uint64_t value = get_reg_by_id(reg_id);
			double f_value = *reinterpret_cast<double *>(&value);
			set_reg_by_id(reg_id, static_cast<int64_t>(f_value));
			break;
		}

		case CMP_INT_8:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();

			int8_t value_1 = static_cast<int8_t>(get_reg_by_id(reg_id_1));
			int8_t value_2 = static_cast<int8_t>(get_reg_by_id(reg_id_2));

			if (value_1 > value_2)
			{
				greater_flag = true;
				equal_flag   = false;
			}
			else if (value_1 == value_2)
			{
				greater_flag = false;
				equal_flag   = true;
			}
			else
			{
				greater_flag = false;
				equal_flag   = false;
			}

			break;
		}

		case CMP_INT_8_U:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();

			uint8_t value_1 = static_cast<uint8_t>(get_reg_by_id(reg_id_1));
			uint8_t value_2 = static_cast<uint8_t>(get_reg_by_id(reg_id_2));

			if (value_1 > value_2)
			{
				greater_flag = true;
				equal_flag   = false;
			}
			else if (value_1 == value_2)
			{
				greater_flag = false;
				equal_flag   = true;
			}
			else
			{
				greater_flag = false;
				equal_flag   = false;
			}

			break;
		}

		case CMP_INT_16:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();

			int16_t value_1 = static_cast<int16_t>(get_reg_by_id(reg_id_1));
			int16_t value_2 = static_cast<int16_t>(get_reg_by_id(reg_id_2));

			if (value_1 > value_2)
			{
				greater_flag = true;
				equal_flag   = false;
			}
			else if (value_1 == value_2)
			{
				greater_flag = false;
				equal_flag   = true;
			}
			else
			{
				greater_flag = false;
				equal_flag   = false;
			}

			break;
		}

		case CMP_INT_16_U:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();

			uint16_t value_1 = static_cast<uint16_t>(get_reg_by_id(reg_id_1));
			uint16_t value_2 = static_cast<uint16_t>(get_reg_by_id(reg_id_2));

			if (value_1 > value_2)
			{
				greater_flag = true;
				equal_flag   = false;
			}
			else if (value_1 == value_2)
			{
				greater_flag = false;
				equal_flag   = true;
			}
			else
			{
				greater_flag = false;
				equal_flag   = false;
			}

			break;
		}

		case CMP_INT_32:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();

			int32_t value_1 = static_cast<int32_t>(get_reg_by_id(reg_id_1));
			int32_t value_2 = static_cast<int32_t>(get_reg_by_id(reg_id_2));

			if (value_1 > value_2)
			{
				greater_flag = true;
				equal_flag   = false;
			}
			else if (value_1 == value_2)
			{
				greater_flag = false;
				equal_flag   = true;
			}
			else
			{
				greater_flag = false;
				equal_flag   = false;
			}

			break;
		}

		case CMP_INT_32_U:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();

			uint32_t value_1 = static_cast<uint32_t>(get_reg_by_id(reg_id_1));
			uint32_t value_2 = static_cast<uint32_t>(get_reg_by_id(reg_id_2));

			if (value_1 > value_2)
			{
				greater_flag = true;
				equal_flag   = false;
			}
			else if (value_1 == value_2)
			{
				greater_flag = false;
				equal_flag   = true;
			}
			else
			{
				greater_flag = false;
				equal_flag   = false;
			}

			break;
		}

		case CMP_INT_64:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();

			int64_t value_1 = static_cast<int64_t>(get_reg_by_id(reg_id_1));
			int64_t value_2 = static_cast<int64_t>(get_reg_by_id(reg_id_2));

			if (value_1 > value_2)
			{
				greater_flag = true;
				equal_flag   = false;
			}
			else if (value_1 == value_2)
			{
				greater_flag = false;
				equal_flag   = true;
			}
			else
			{
				greater_flag = false;
				equal_flag   = false;
			}

			break;
		}

		case CMP_INT_64_U:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();

			uint64_t value_1 = static_cast<uint64_t>(get_reg_by_id(reg_id_1));
			uint64_t value_2 = static_cast<uint64_t>(get_reg_by_id(reg_id_2));

			if (value_1 > value_2)
			{
				greater_flag = true;
				equal_flag   = false;
			}
			else if (value_1 == value_2)
			{
				greater_flag = false;
				equal_flag   = true;
			}
			else
			{
				greater_flag = false;
				equal_flag   = false;
			}

			break;
		}

		case CMP_FLT_32:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();

			uint32_t value_1 = static_cast<uint32_t>(get_reg_by_id(reg_id_1));
			uint32_t value_2 = static_cast<uint32_t>(get_reg_by_id(reg_id_2));

			float f_value_1 = *reinterpret_cast<float *>(&value_1);
			float f_value_2 = *reinterpret_cast<float *>(&value_2);

			if (f_value_1 > f_value_2)
			{
				greater_flag = true;
				equal_flag   = false;
			}
			else if (f_value_1 == f_value_2)
			{
				greater_flag = false;
				equal_flag   = true;
			}
			else
			{
				greater_flag = false;
				equal_flag   = false;
			}

			break;
		}

		case CMP_FLT_64:
		{
			uint8_t reg_id_1 = fetch<uint8_t>();
			uint8_t reg_id_2 = fetch<uint8_t>();

			uint64_t value_1 = get_reg_by_id(reg_id_1);
			uint64_t value_2 = get_reg_by_id(reg_id_2);

			double f_value_1 = *reinterpret_cast<double *>(&value_1);
			double f_value_2 = *reinterpret_cast<double *>(&value_2);

			if (f_value_1 > f_value_2)
			{
				greater_flag = true;
				equal_flag   = false;
			}
			else if (f_value_1 == f_value_2)
			{
				greater_flag = false;
				equal_flag   = true;
			}
			else
			{
				greater_flag = false;
				equal_flag   = false;
			}

			break;
		}

		case SET_IF_GT:
		{
			uint8_t reg_id = fetch<uint8_t>();
			if (greater_flag)
				set_reg_by_id(reg_id, 1);
			else
				set_reg_by_id(reg_id, 0);
			break;
		}

		case SET_IF_GEQ:
		{
			uint8_t reg_id = fetch<uint8_t>();
			if (greater_flag | equal_flag)
				set_reg_by_id(reg_id, 1);
			else
				set_reg_by_id(reg_id, 0);
			break;
		}

		case SET_IF_LT:
		{
			uint8_t reg_id = fetch<uint8_t>();
			if (!greater_flag & !equal_flag)
				set_reg_by_id(reg_id, 1);
			else
				set_reg_by_id(reg_id, 0);
			break;
		}

		case SET_IF_LEQ:
		{
			uint8_t reg_id = fetch<uint8_t>();
			if (!greater_flag)
				set_reg_by_id(reg_id, 1);
			else
				set_reg_by_id(reg_id, 0);
			break;
		}

		case SET_IF_EQ:
		{
			uint8_t reg_id = fetch<uint8_t>();
			if (equal_flag)
				set_reg_by_id(reg_id, 1);
			else
				set_reg_by_id(reg_id, 0);
			break;
		}

		case SET_IF_NEQ:
		{
			uint8_t reg_id = fetch<uint8_t>();
			if (!equal_flag)
				set_reg_by_id(reg_id, 1);
			else
				set_reg_by_id(reg_id, 0);
			break;
		}

		case JUMP:
		{
			int64_t offset = fetch<int64_t>();
			jump_instruction_p(offset);
			break;
		}

		case JUMP_IF_GT:
		{
			int64_t offset = fetch<int64_t>();
			if (greater_flag)
				jump_instruction_p(offset);
			break;
		}

		case JUMP_IF_GEQ:
		{
			int64_t offset = fetch<int64_t>();
			if (greater_flag | equal_flag)
				jump_instruction_p(offset);
			break;
		}

		case JUMP_IF_LT:
		{
			int64_t offset = fetch<int64_t>();
			if (!greater_flag & !equal_flag)
				jump_instruction_p(offset);
			break;
		}

		case JUMP_IF_LEQ:
		{
			int64_t offset = fetch<int64_t>();
			if (!greater_flag)
				jump_instruction_p(offset);
			break;
		}

		case JUMP_IF_EQ:
		{
			int64_t offset = fetch<int64_t>();
			if (equal_flag)
				jump_instruction_p(offset);
			break;
		}

		case JUMP_IF_NEQ:
		{
			int64_t offset = fetch<int64_t>();
			if (!equal_flag)
				jump_instruction_p(offset);
			break;
		}

		case PUSH_REG_8:
		{
			uint8_t reg_id = fetch<uint8_t>();
			uint8_t value  = get_reg_by_id(reg_id);
			push(value);
			break;
		}

		case PUSH_REG_16:
		{
			uint8_t reg_id = fetch<uint8_t>();
			uint16_t value = get_reg_by_id(reg_id);
			push(value);
			break;
		}

		case PUSH_REG_32:
		{
			uint8_t reg_id = fetch<uint8_t>();
			uint32_t value = get_reg_by_id(reg_id);
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
			uint8_t value  = pop<uint8_t>();
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
			int64_t offset = fetch<int64_t>();
			push_stack_frame();
			jump_instruction_p(offset);
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
			break;
		}

		case DEALLOCATE_STACK:
		{
			uint64_t size = fetch<uint64_t>();
			regs[R_STACK_PTR] -= size;
			break;
		}

		case COMMENT:
		case LABEL:
		{
			while (fetch<uint8_t>() != '\0')
				;
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
			int16_t c      = getc(stdin);
			set_reg_by_id(reg_id, c);
			break;
		}
		}
	}
};

#endif
