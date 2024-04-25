#ifndef TEA_ASSEMBLER_HEADER
#define TEA_ASSEMBLER_HEADER

#include <unordered_map>

#include "../VM/cpu.hpp"
#include "byte_code.hpp"
#include "buffer-builder.hpp"
#include "buffer.hpp"

/**
 * @brief Structure containing the location to an entry of static data.
 * The static data segment is prepended to the program and can be accessed and
 * modified during runtime.
 */
struct StaticData
{
	/**
	 * @brief The offset of the entry in the static data segment.
	 */
	uint64_t offset;

	/**
	 * @brief The size of the entry in the static data segment.
	 */
	uint64_t size;

	/**
	 * @brief Computes the offset to the next entry in the
	 * static data segment.
	 */
	uint64_t
	end()
	{
		return offset + size;
	}
};

/**
 * @brief The assembler class.
 * This class is used in the translation of a program into the byte code.
 * It also contains all labels and static data entries.
 * It contains methods to add labels and static data entries,
 * add instructions to the program, allocate registers,
 * and to translate the program into the byte code.
 */
struct Assembler : public BufferBuilder
{
	/**
	 * @brief Map containing all labels.
	 * The key is the label name and the value is the offset
	 * of the label in the program.
	 */
	std::unordered_map<std::string /* id */, uint64_t /* position */> labels;

	/**
	 * @brief Map containing all references to labels.
	 * The key is the label name and the value is a list of
	 * positions in the program where the label is referenced.
	 */
	std::unordered_map<std::string /* id */, std::vector<uint64_t>> label_references;

	/**
	 * @brief Bit array used to check whether a register is free.
	 */
	std::vector<bool> free_registers;

	/**
	 * @brief A buffer builder for the static data segment.
	 * It is later built and prepended to the program.
	 */
	BufferBuilder static_data;

	/**
	 * @brief Construct a new Assembler object.
	 */
	Assembler()
		: free_registers(CPU::general_purpose_register_count, true) {}

	/**
	 * @brief Destroy the Assembler object.
	 * The program and static data buffers are freed.
	 */
	~Assembler()
	{
		free_buffer();
		static_data.free_buffer();
	}

	/**
	 * @brief Reserves a register that can be used in instructions.
	 */
	uint8_t
	get_register()
	{
		// Find the first free register

		for (uint8_t i = 0; i < CPU::general_purpose_register_count; i++)
		{
			if (free_registers[i])
			{
				free_registers[i] = false;
				return i;
			}
		}

		// Todo: create

		fprintf(stderr, "No free registers found. Spilling registers is not implemented yet.");
		abort();
	}

	/**
	 * @brief Frees a register.
	 * This allows it to be used again.
	 * @param register_id The register to free.
	 */
	void
	free_register(uint8_t reg_id)
	{
		free_registers[reg_id] = true;
	}

	/**
	 * @brief Assembles the program into byte code.
	 * @returns A buffer containing the byte code.
	 */
	Buffer
	assemble()
	{
		BufferBuilder executable;
		update_label_references();

		// Push the size of the static data segment
		// and the size of the program instructions.

		executable.push<uint64_t>(static_data.offset);
		executable.push<uint64_t>(offset);

		// Combine static data and program instructions.

		for (size_t j = 0; j < static_data.offset; j++)
		{
			executable.push(static_data[j]);
		}

		for (size_t j = 0; j < offset; j++)
		{
			executable.push(operator[](j));
		}

		// Create memory from the program and return it.

		return executable.build();
	}

	/**
	 * @brief Pushes an instruction onto the program buffer
	 * @param instruction The instruction to add.
	 */
	void
	push_instruction(enum Instruction instruction)
	{
		push<uint16_t>(instruction);
	}

	/**
	 * @brief Adds a MOVE_8_INTO_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	move_8_into_reg(uint8_t lit, uint8_t reg_id)
	{
		push_instruction(MOVE_8_INTO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a MOVE_16_INTO_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	move_16_into_reg(uint16_t lit, uint8_t reg_id)
	{
		push_instruction(MOVE_16_INTO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a MOVE_32_INTO_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	move_32_into_reg(uint32_t lit, uint8_t reg_id)
	{
		push_instruction(MOVE_32_INTO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a MOVE_64_INTO_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	move_64_into_reg(uint64_t lit, uint8_t reg_id)
	{
		push_instruction(MOVE_64_INTO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a MOVE_8_INTO_MEM instruction to the program.
	 * @param lit The source literal.
	 * @param mem_id The destination memory address.
	 */
	void
	move_8_into_mem(uint8_t lit, uint64_t address)
	{
		push_instruction(MOVE_8_INTO_MEM);
		push(lit);
		push(address);
	}

	/**
	 * @brief Adds a MOVE_16_INTO_MEM instruction to the program.
	 * @param lit The source literal.
	 * @param mem_id The destination memory address.
	 */
	void
	move_16_into_mem(uint16_t lit, uint64_t address)
	{
		push_instruction(MOVE_16_INTO_MEM);
		push(lit);
		push(address);
	}

	/**
	 * @brief Adds a MOVE_32_INTO_MEM instruction to the program.
	 * @param lit The source literal.
	 * @param mem_id The destination memory address.
	 */
	void
	move_32_into_mem(uint32_t lit, uint64_t address)
	{
		push_instruction(MOVE_32_INTO_MEM);
		push(lit);
		push(address);
	}

	/**
	 * @brief Adds a MOVE_64_INTO_MEM instruction to the program.
	 * @param lit The source literal.
	 * @param mem_id The destination memory address.
	 */
	void
	move_64_into_mem(uint64_t lit, uint64_t address)
	{
		push_instruction(MOVE_64_INTO_MEM);
		push(lit);
		push(address);
	}

	/**
	 * @brief Adds a MOVE_REG_INTO_REG instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	move_reg_into_reg(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(MOVE_REG_INTO_REG);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a MOVE_REG_INTO_MEM_8 instruction to the program.
	 * @param reg_id The source register.
	 * @param address The destination memory address.
	 */
	void
	move_reg_into_mem_8(uint8_t reg_id, uint64_t address)
	{
		push_instruction(MOVE_REG_INTO_MEM_8);
		push(reg_id);
		push(address);
	}

	/**
	 * @brief Adds a MOVE_REG_INTO_MEM_16 instruction to the program.
	 * @param reg_id The source register.
	 * @param address The destination memory address.
	 */
	void
	move_reg_into_mem_16(uint8_t reg_id, uint64_t address)
	{
		push_instruction(MOVE_REG_INTO_MEM_16);
		push(reg_id);
		push(address);
	}

	/**
	 * @brief Adds a MOVE_REG_INTO_MEM_32 instruction to the program.
	 * @param reg_id The source register.
	 * @param address The destination memory address.
	 */
	void
	move_reg_into_mem_32(uint8_t reg_id, uint64_t address)
	{
		push_instruction(MOVE_REG_INTO_MEM_32);
		push(reg_id);
		push(address);
	}

	/**
	 * @brief Adds a MOVE_REG_INTO_MEM_64 instruction to the program.
	 * @param reg_id The source register.
	 * @param address The destination memory address.
	 */
	void
	move_reg_into_mem_64(uint8_t reg_id, uint64_t address)
	{
		push_instruction(MOVE_REG_INTO_MEM_64);
		push(reg_id);
		push(address);
	}

	/**
	 * @brief Adds a MOVE_MEM_8_INTO_REG instruction to the program.
	 * @param address The source memory address.
	 * @param reg_id The destination register.
	 */
	void
	move_mem_8_into_reg(uint64_t address, uint8_t reg_id)
	{
		push_instruction(MOVE_MEM_8_INTO_REG);
		push(address);
		push(reg_id);
	}

	/**
	 * @brief Adds a MOVE_MEM_16_INTO_REG instruction to the program.
	 * @param address The source memory address.
	 * @param reg_id The destination register.
	 */
	void
	move_mem_16_into_reg(uint64_t address, uint8_t reg_id)
	{
		push_instruction(MOVE_MEM_16_INTO_REG);
		push(address);
		push(reg_id);
	}

	/**
	 * @brief Adds a MOVE_MEM_32_INTO_REG instruction to the program.
	 * @param address The source memory address.
	 * @param reg_id The destination register.
	 */
	void
	move_mem_32_into_reg(uint64_t address, uint8_t reg_id)
	{
		push_instruction(MOVE_MEM_32_INTO_REG);
		push(address);
		push(reg_id);
	}

	/**
	 * @brief Adds a MOVE_MEM_64_INTO_REG instruction to the program.
	 * @param address The source memory address.
	 * @param reg_id The destination register.
	 */
	void
	move_mem_64_into_reg(uint64_t address, uint8_t reg_id)
	{
		push_instruction(MOVE_MEM_64_INTO_REG);
		push(address);
		push(reg_id);
	}

	/**
	 * @brief Adds a MOVE_REG_POINTER_8_INTO_REG instruction to the program.
	 * @param reg_id_1 The source register that holds a pointer.
	 * @param reg_id_2 The destination register.
	 */
	void
	move_reg_pointer_8_into_reg(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(MOVE_REG_POINTER_8_INTO_REG);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a MOVE_REG_POINTER_16_INTO_REG instruction to the program.
	 * @param reg_id_1 The source register that holds a pointer.
	 * @param reg_id_2 The destination register.
	 */
	void
	move_reg_pointer_16_into_reg(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(MOVE_REG_POINTER_16_INTO_REG);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a MOVE_REG_POINTER_32_INTO_REG instruction to the program.
	 * @param reg_id_1 The source register that holds a pointer.
	 * @param reg_id_2 The destination register.
	 */
	void
	move_reg_pointer_32_into_reg(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(MOVE_REG_POINTER_32_INTO_REG);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a MOVE_REG_POINTER_64_INTO_REG instruction to the program.
	 * @param reg_id_1 The source register that holds a pointer.
	 * @param reg_id_2 The destination register.
	 */
	void
	move_reg_pointer_64_into_reg(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(MOVE_REG_POINTER_64_INTO_REG);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a MOVE_REG_INTO_REG_POINTER_8 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register that holds a pointer.
	 */
	void
	move_reg_into_reg_pointer_8(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(MOVE_REG_INTO_REG_POINTER_8);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a MOVE_REG_INTO_REG_POINTER_16 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register that holds a pointer.
	 */
	void
	move_reg_into_reg_pointer_16(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(MOVE_REG_INTO_REG_POINTER_16);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a MOVE_REG_INTO_REG_POINTER_32 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register that holds a pointer.
	 */
	void
	move_reg_into_reg_pointer_32(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(MOVE_REG_INTO_REG_POINTER_32);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a MOVE_REG_INTO_REG_POINTER_64 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register that holds a pointer.
	 */
	void
	move_reg_into_reg_pointer_64(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(MOVE_REG_INTO_REG_POINTER_64);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a MOVE_FRAME_OFFSET_8_INTO_REG instruction to the program.
	 * @param offset The source offset to the start of the stack frame.
	 * @param reg_id The destination register.
	 */
	void
	move_frame_offset_8_into_reg(int64_t offset, uint8_t reg_id)
	{
		push_instruction(MOVE_FRAME_OFFSET_8_INTO_REG);
		push(offset);
		push(reg_id);
	}

	/**
	 * @brief Adds a MOVE_FRAME_OFFSET_16_INTO_REG instruction to the program.
	 * @param offset The source offset to the start of the stack frame.
	 * @param reg_id The destination register.
	 */
	void
	move_frame_offset_16_into_reg(int64_t offset, uint8_t reg_id)
	{
		push_instruction(MOVE_FRAME_OFFSET_16_INTO_REG);
		push(offset);
		push(reg_id);
	}

	/**
	 * @brief Adds a MOVE_FRAME_OFFSET_32_INTO_REG instruction to the program.
	 * @param offset The source offset to the start of the stack frame.
	 * @param reg_id The destination register.
	 */
	void
	move_frame_offset_32_into_reg(int64_t offset, uint8_t reg_id)
	{
		push_instruction(MOVE_FRAME_OFFSET_32_INTO_REG);
		push(offset);
		push(reg_id);
	}

	/**
	 * @brief Adds a MOVE_FRAME_OFFSET_64_INTO_REG instruction to the program.
	 * @param offset The source offset to the start of the stack frame.
	 * @param reg_id The destination register.
	 */
	void
	move_frame_offset_64_into_reg(int64_t offset, uint8_t reg_id)
	{
		push_instruction(MOVE_FRAME_OFFSET_64_INTO_REG);
		push(offset);
		push(reg_id);
	}

	/**
	 * @brief Adds a MOVE_REG_INTO_FRAME_OFFSET_8 instruction to the program.
	 * @param reg_id The source register.
	 * @param offset The destination offset to the start of the stack frame.
	 */
	void
	move_reg_into_frame_offset_8(uint8_t reg_id, int64_t offset)
	{
		push_instruction(MOVE_REG_INTO_FRAME_OFFSET_8);
		push(reg_id);
		push(offset);
	}

	/**
	 * @brief Adds a MOVE_REG_INTO_FRAME_OFFSET_16 instruction to the program.
	 * @param reg_id The source register.
	 * @param offset The destination offset to the start of the stack frame.
	 */
	void
	move_reg_into_frame_offset_16(uint8_t reg_id, int64_t offset)
	{
		push_instruction(MOVE_REG_INTO_FRAME_OFFSET_32);
		push(reg_id);
		push(offset);
	}

	/**
	 * @brief Adds a MOVE_REG_INTO_FRAME_OFFSET_32 instruction to the program.
	 * @param reg_id The source register.
	 * @param offset The destination offset to the start of the stack frame.
	 */
	void
	move_reg_into_frame_offset_32(uint8_t reg_id, int64_t offset)
	{
		push_instruction(MOVE_REG_INTO_FRAME_OFFSET_32);
		push(reg_id);
		push(offset);
	}

	/**
	 * @brief Adds a MOVE_REG_INTO_FRAME_OFFSET_64 instruction to the program.
	 * @param reg_id The source register.
	 * @param offset The destination offset to the start of the stack frame.
	 */
	void
	move_reg_into_frame_offset_64(uint8_t reg_id, int64_t offset)
	{
		push_instruction(MOVE_REG_INTO_FRAME_OFFSET_64);
		push(reg_id);
		push(offset);
	}

	/**
	 * @brief Adds a MOVE_STACK_TOP_OFFSET_8_INTO_REG instruction to the program.
	 * @param offset The source offset to the top of the stack.
	 * @param reg_id The destination register.
	 */
	void
	move_stack_top_offset_8_into_reg(int64_t offset, uint8_t reg_id)
	{
		push_instruction(MOVE_STACK_TOP_OFFSET_8_INTO_REG);
		push(offset);
		push(reg_id);
	}

	/**
	 * @brief Adds a MOVE_STACK_TOP_OFFSET_16_INTO_REG instruction to the program.
	 * @param offset The source offset to the top of the stack.
	 * @param reg_id The destination register.
	 */
	void
	move_stack_top_offset_16_into_reg(int64_t offset, uint8_t reg_id)
	{
		push_instruction(MOVE_STACK_TOP_OFFSET_16_INTO_REG);
		push(offset);
		push(reg_id);
	}

	/**
	 * @brief Adds a MOVE_STACK_TOP_OFFSET_32_INTO_REG instruction to the program.
	 * @param offset The source offset to the top of the stack.
	 * @param reg_id The destination register.
	 */
	void
	move_stack_top_offset_32_into_reg(int64_t offset, uint8_t reg_id)
	{
		push_instruction(MOVE_STACK_TOP_OFFSET_32_INTO_REG);
		push(offset);
		push(reg_id);
	}

	/**
	 * @brief Adds a MOVE_STACK_TOP_OFFSET_64_INTO_REG instruction to the program.
	 * @param offset The source offset to the top of the stack.
	 * @param reg_id The destination register.
	 */
	void
	move_stack_top_offset_64_into_reg(int64_t offset, uint8_t reg_id)
	{
		push_instruction(MOVE_STACK_TOP_OFFSET_64_INTO_REG);
		push(offset);
		push(reg_id);
	}

	/**
	 * @brief Adds a MOVE_REG_INTO_STACK_TOP_OFFSET_8 instruction to the program.
	 * @param reg_id The source register.
	 * @param offset The destination offset to the top of the stack.
	 */
	void
	move_reg_into_stack_top_offset_8(uint8_t reg_id, int64_t offset)
	{
		push_instruction(MOVE_REG_INTO_STACK_TOP_OFFSET_8);
		push(reg_id);
		push(offset);
	}

	/**
	 * @brief Adds a MOVE_REG_INTO_STACK_TOP_OFFSET_16 instruction to the program.
	 * @param reg_id The source register.
	 * @param offset The destination offset to the top of the stack.
	 */
	void
	move_reg_into_stack_top_offset_16(uint8_t reg_id, int64_t offset)
	{
		push_instruction(MOVE_REG_INTO_STACK_TOP_OFFSET_32);
		push(reg_id);
		push(offset);
	}

	/**
	 * @brief Adds a MOVE_REG_INTO_STACK_TOP_OFFSET_32 instruction to the program.
	 * @param reg_id The source register.
	 * @param offset The destination offset to the top of the stack.
	 */
	void
	move_reg_into_stack_top_offset_32(uint8_t reg_id, int64_t offset)
	{
		push_instruction(MOVE_REG_INTO_STACK_TOP_OFFSET_32);
		push(reg_id);
		push(offset);
	}

	/**
	 * @brief Adds a MOVE_REG_INTO_STACK_TOP_OFFSET_64 instruction to the program.
	 * @param reg_id The source register.
	 * @param offset The destination offset to the top of the stack.
	 */
	void
	move_reg_into_stack_top_offset_64(uint8_t reg_id, int64_t offset)
	{
		push_instruction(MOVE_REG_INTO_STACK_TOP_OFFSET_64);
		push(reg_id);
		push(offset);
	}

	/**
	 * @brief Adds a MOVE_STACK_TOP_ADDRESS_INTO_REG instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	move_stack_top_address_into_reg(uint8_t reg_id)
	{
		push_instruction(MOVE_STACK_TOP_ADDRESS_INTO_REG);
		push(reg_id);
	}

	/**
	 * @brief Adds a ADD_8_INTO_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	add_8_into_reg(uint8_t lit, uint8_t reg_id)
	{
		push_instruction(ADD_8_INTO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a ADD_16_INTO_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	add_16_into_reg(uint16_t lit, uint8_t reg_id)
	{
		push_instruction(ADD_16_INTO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a ADD_32_INTO_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	add_32_into_reg(uint32_t lit, uint8_t reg_id)
	{
		push_instruction(ADD_32_INTO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a ADD_64_INTO_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	add_64_into_reg(uint64_t lit, uint8_t reg_id)
	{
		push_instruction(ADD_64_INTO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a ADD_REG_INTO_REG instruction to the program.
	 * @param lit The source register.
	 * @param reg_id The destination register.
	 */
	void
	add_reg_into_reg(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(ADD_REG_INTO_REG);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a SUBTRACT_8_FROM_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	subtract_8_from_reg(uint8_t lit, uint8_t reg_id)
	{
		push_instruction(SUBTRACT_8_FROM_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a SUBTRACT_16_FROM_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	subtract_16_from_reg(uint16_t lit, uint8_t reg_id)
	{
		push_instruction(SUBTRACT_16_FROM_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a SUBTRACT_32_FROM_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	subtract_32_from_reg(uint32_t lit, uint8_t reg_id)
	{
		push_instruction(SUBTRACT_32_FROM_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a SUBTRACT_64_FROM_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	subtract_64_from_reg(uint64_t lit, uint8_t reg_id)
	{
		push_instruction(SUBTRACT_64_FROM_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a SUBTRACT_REG_FROM_REG instruction to the program.
	 * @param lit The source register.
	 * @param reg_id The destination register.
	 */
	void
	subtract_reg_from_reg(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(SUBTRACT_REG_FROM_REG);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a MULTIPLY_8_INTO_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	multiply_8_into_reg(uint8_t lit, uint8_t reg_id)
	{
		push_instruction(MULTIPLY_8_INTO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a MULTIPLY_16_INTO_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	multiply_16_into_reg(uint16_t lit, uint8_t reg_id)
	{
		push_instruction(MULTIPLY_16_INTO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a MULTIPLY_32_INTO_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	multiply_32_into_reg(uint32_t lit, uint8_t reg_id)
	{
		push_instruction(MULTIPLY_32_INTO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a MULTIPLY_64_INTO_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	multiply_64_into_reg(uint64_t lit, uint8_t reg_id)
	{
		push_instruction(MULTIPLY_64_INTO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a MULTIPLY_REG_INTO_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	multiply_reg_into_reg(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(MULTIPLY_REG_INTO_REG);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a DIVIDE_8_FROM_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	divide_8_from_reg(uint8_t lit, uint8_t reg_id)
	{
		push_instruction(DIVIDE_8_FROM_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a DIVIDE_16_FROM_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	divide_16_from_reg(uint16_t lit, uint8_t reg_id)
	{
		push_instruction(DIVIDE_16_FROM_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a DIVIDE_32_FROM_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	divide_32_from_reg(uint32_t lit, uint8_t reg_id)
	{
		push_instruction(DIVIDE_32_FROM_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a DIVIDE_64_FROM_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	divide_64_from_reg(uint64_t lit, uint8_t reg_id)
	{
		push_instruction(DIVIDE_64_FROM_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a DIVIDE_REG_INTO_REG instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	divide_reg_from_reg(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(DIVIDE_REG_FROM_REG);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a TAKE_MODULO_8_FROM_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	take_modulo_8_of_reg(uint8_t lit, uint8_t reg_id)
	{
		push_instruction(TAKE_MODULO_8_OF_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a TAKE_MODULO_16_FROM_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	take_modulo_16_of_reg(uint16_t lit, uint8_t reg_id)
	{
		push_instruction(TAKE_MODULO_16_OF_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a TAKE_MODULO_32_FROM_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	take_modulo_32_of_reg(uint32_t lit, uint8_t reg_id)
	{
		push_instruction(TAKE_MODULO_32_OF_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a TAKE_MODULO_64_FROM_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	take_modulo_64_of_reg(uint64_t lit, uint8_t reg_id)
	{
		push_instruction(TAKE_MODULO_64_OF_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a TAKE_MODULO_REG_OF_REG instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	take_modulo_reg_of_reg(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(TAKE_MODULO_REG_OF_REG);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a AND_8_INTO_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	and_8_into_reg(uint8_t lit, uint8_t reg_id)
	{
		push_instruction(AND_8_INTO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a AND_16_INTO_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	and_16_into_reg(uint16_t lit, uint8_t reg_id)
	{
		push_instruction(AND_16_INTO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a AND_32_INTO_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	and_32_into_reg(uint32_t lit, uint8_t reg_id)
	{
		push_instruction(AND_32_INTO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a AND_64_INTO_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	and_64_into_reg(uint64_t lit, uint8_t reg_id)
	{
		push_instruction(AND_64_INTO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a AND_REG_INTO_REG instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	and_reg_into_reg(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(AND_REG_INTO_REG);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a OR_8_INTO_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	or_8_into_reg(uint8_t lit, uint8_t reg_id)
	{
		push_instruction(OR_8_INTO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a OR_16_INTO_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	or_16_into_reg(uint16_t lit, uint8_t reg_id)
	{
		push_instruction(OR_16_INTO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a OR_32_INTO_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	or_32_into_reg(uint32_t lit, uint8_t reg_id)
	{
		push_instruction(OR_32_INTO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a OR_64_INTO_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	or_64_into_reg(uint64_t lit, uint8_t reg_id)
	{
		push_instruction(OR_64_INTO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a OR_REG_INTO_REG instruction to the program.
	 * @param reg_id_1 The source literal.
	 * @param reg_id_2 The destination register.
	 */
	void
	or_reg_into_reg(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(OR_REG_INTO_REG);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a XOR_8_INTO_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	xor_8_into_reg(uint8_t lit, uint8_t reg_id)
	{
		push_instruction(XOR_8_INTO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a XOR_16_INTO_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	xor_16_into_reg(uint16_t lit, uint8_t reg_id)
	{
		push_instruction(XOR_16_INTO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a XOR_32_INTO_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	xor_32_into_reg(uint32_t lit, uint8_t reg_id)
	{
		push_instruction(XOR_32_INTO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a XOR_64_INTO_REG instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	xor_64_into_reg(uint64_t lit, uint8_t reg_id)
	{
		push_instruction(XOR_64_INTO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a XOR_REG_INTO_REG instruction to the program.
	 * @param red_id_1 The source literal.
	 * @param reg_id_2 The destination register.
	 */
	void
	xor_reg_into_reg(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(XOR_REG_INTO_REG);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a LEFT_SHIFT_REG_BY_8 instruction to the program.
	 * @param reg_id The destination register.
	 * @param shift_size The source shift size.
	 */
	void
	left_shift_reg_by_8(uint8_t reg_id, uint8_t shift_size)
	{
		push_instruction(LEFT_SHIFT_REG_BY_8);
		push(reg_id);
		push(shift_size);
	}

	/**
	 * @brief Adds a LEFT_SHIFT_REG_BY_REG instruction to the program.
	 * @param reg_id_1 The destination register.
	 * @param reg_id_2 The source register.
	 */
	void
	left_shift_reg_by_reg(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(LEFT_SHIFT_REG_BY_REG);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a RIGHT_SHIFT_REG_BY_8 instruction to the program.
	 * @param reg_id The destination register.
	 * @param shift_size The source shift size.
	 */
	void
	right_shift_reg_by_8(uint8_t reg_id, uint8_t shift_size)
	{
		push_instruction(RIGHT_SHIFT_REG_BY_8);
		push(reg_id);
		push(shift_size);
	}
	/**
	 * @brief Adds a RIGHT_SHIFT_REG_BY_REG instruction to the program.
	 * @param reg_id_1 The destination register.
	 * @param reg_id_2 The source register.
	 */
	void
	right_shift_reg_by_reg(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(RIGHT_SHIFT_REG_BY_REG);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a INCREMENT_REG instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	increment_reg(uint8_t reg_id)
	{
		push_instruction(INCREMENT_REG);
		push(reg_id);
	}

	/**
	 * @brief Adds a DECREMENT_REG instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	decrement_reg(uint8_t reg_id)
	{
		push_instruction(DECREMENT_REG);
		push(reg_id);
	}

	/**
	 * @brief Adds a NOT_REG instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	not_reg(uint8_t reg_id)
	{
		push_instruction(NOT_REG);
		push(reg_id);
	}

	/**
	 * @brief Adds a COMPARE_8_TO_REG instruction to the program.
	 * @param lit The LHS literal.
	 * @param reg_id The RHS register.
	 */
	void
	compare_8_to_reg(uint8_t lit, uint8_t reg_id)
	{
		push_instruction(COMPARE_8_TO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a COMPARE_16_TO_REG instruction to the program.
	 * @param lit The LHS literal.
	 * @param reg_id The RHS register.
	 */
	void
	compare_16_to_reg(uint16_t lit, uint8_t reg_id)
	{
		push_instruction(COMPARE_16_TO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a COMPARE_32_TO_REG instruction to the program.
	 * @param lit The LHS literal.
	 * @param reg_id The RHS register.
	 */
	void
	compare_32_to_reg(uint32_t lit, uint8_t reg_id)
	{
		push_instruction(COMPARE_32_TO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a COMPARE_64_TO_REG instruction to the program.
	 * @param lit The LHS literal.
	 * @param reg_id The RHS register.
	 */
	void
	compare_64_to_reg(uint64_t lit, uint8_t reg_id)
	{
		push_instruction(COMPARE_64_TO_REG);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a COMPARE_REG_TO_8 instruction to the program.
	 * @param reg_id The LHS register.
	 * @param lit The RHS literal.
	 */
	void
	compare_reg_to_8(uint8_t reg_id, uint8_t lit)
	{
		push_instruction(COMPARE_REG_TO_8);
		push(reg_id);
		push(lit);
	}

	/**
	 * @brief Adds a COMPARE_REG_TO_16 instruction to the program.
	 * @param reg_id The LHS register.
	 * @param lit The RHS literal.
	 */
	void
	compare_reg_to_16(uint8_t reg_id, uint16_t lit)
	{
		push_instruction(COMPARE_REG_TO_16);
		push(reg_id);
		push(lit);
	}

	/**
	 * @brief Adds a COMPARE_REG_TO_32 instruction to the program.
	 * @param reg_id The LHS register.
	 * @param lit The RHS literal.
	 */
	void
	compare_reg_to_32(uint8_t reg_id, uint32_t lit)
	{
		push_instruction(COMPARE_REG_TO_32);
		push(reg_id);
		push(lit);
	}

	/**
	 * @brief Adds a COMPARE_REG_TO_64 instruction to the program.
	 * @param reg_id The LHS register.
	 * @param lit The RHS literal.
	 */
	void
	compare_reg_to_64(uint8_t reg_id, uint64_t lit)
	{
		push_instruction(COMPARE_REG_TO_64);
		push(reg_id);
		push(lit);
	}

	/**
	 * @brief Adds a COMPARE_REG_TO_REG_SIGNED instruction to the program.
	 * @param reg_id The LHS register.
	 * @param reg_id The RHS register.
	 */
	void
	compare_reg_to_reg_signed(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(COMPARE_REG_TO_REG_SIGNED);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a COMPARE_REG_TO_REG_UNSIGNED instruction to the program.
	 * @param reg_id The LHS register.
	 * @param reg_id The RHS register.
	 */
	void
	compare_reg_to_reg_unsigned(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(COMPARE_REG_TO_REG_UNSIGNED);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a SET_REG_IF_GREATER instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	set_reg_if_greater(uint8_t reg_id)
	{
		push_instruction(SET_REG_IF_GREATER);
		push(reg_id);
	}

	/**
	 * @brief Adds a SET_REG_IF_GREATER_OR_EQUAL instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	set_reg_if_greater_or_equal(uint8_t reg_id)
	{
		push_instruction(SET_REG_IF_GREATER_OR_EQUAL);
		push(reg_id);
	}

	/**
	 * @brief Adds a SET_REG_IF_LESS instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	set_reg_if_less(uint8_t reg_id)
	{
		push_instruction(SET_REG_IF_LESS);
		push(reg_id);
	}

	/**
	 * @brief Adds a SET_REG_IF_LESS_OR_EQUAL instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	set_reg_if_less_or_equal(uint8_t reg_id)
	{
		push_instruction(SET_REG_IF_LESS_OR_EQUAL);
		push(reg_id);
	}

	/**
	 * @brief Adds a SET_REG_IF_EQUAL instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	set_reg_if_equal(uint8_t reg_id)
	{
		push_instruction(SET_REG_IF_EQUAL);
		push(reg_id);
	}

	/**
	 * @brief Adds a SET_REG_IF_NOT_EQUAL instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	set_reg_if_not_equal(uint8_t reg_id)
	{
		push_instruction(SET_REG_IF_NOT_EQUAL);
		push(reg_id);
	}

	/**
	 * @brief Adds a JUMP instruction to the program.
	 * @param label The label to jump to.
	 */
	void
	jump(const std::string &label)
	{
		push_instruction(JUMP);
		add_label_reference(label);
		push<uint64_t>(0); // This will be updated later
	}

	/**
	 * @brief Adds a JUMP_IF_GREATER instruction to the program.
	 * @param label The label to jump to.
	 */
	void
	jump_if_greater(const std::string &label)
	{
		push_instruction(JUMP_IF_GREATER);
		add_label_reference(label);
		push<uint64_t>(0); // This will be updated later
	}

	/**
	 * @brief Adds a JUMP_IF_GREATER_OR_EQUAL instruction to the program.
	 * @param label The label to jump to.
	 */
	void
	jump_if_greater_or_equal(const std::string &label)
	{
		push_instruction(JUMP_IF_GREATER_OR_EQUAL);
		add_label_reference(label);
		push<uint64_t>(0); // This will be updated later
	}

	/**
	 * @brief Adds a JUMP_IF_LESS instruction to the program.
	 * @param label The label to jump to.
	 */
	void
	jump_if_less(const std::string &label)
	{
		push_instruction(JUMP_IF_LESS);
		add_label_reference(label);
		push<uint64_t>(0); // This will be updated later
	}

	/**
	 * @brief Adds a JUMP_IF_LESS_OR_EQUAL instruction to the program.
	 * @param label The label to jump to.
	 */
	void
	jump_if_less_or_equal(const std::string &label)
	{
		push_instruction(JUMP_IF_LESS_OR_EQUAL);
		add_label_reference(label);
		push<uint64_t>(0); // This will be updated later
	}

	/**
	 * @brief Adds a JUMP_IF_EQUAL instruction to the program.
	 * @param label The label to jump to.
	 */
	void
	jump_if_equal(const std::string &label)
	{
		push_instruction(JUMP_IF_EQUAL);
		add_label_reference(label);
		push<uint64_t>(0); // This will be updated later
	}

	/**
	 * @brief Adds a JUMP_IF_NOT_EQUAL instruction to the program.
	 * @param label The label to jump to.
	 */
	void
	jump_if_not_equal(const std::string &label)
	{
		push_instruction(JUMP_IF_NOT_EQUAL);
		add_label_reference(label);
		push<uint64_t>(0); // This will be updated later
	}

	/**
	 * @brief Adds a PUSH_8 instruction to the program.
	 * @param lit The source literal to push.
	 */
	void
	push_8(uint8_t lit)
	{
		push_instruction(PUSH_8);
		push(lit);
	}

	/**
	 * @brief Adds a PUSH_16 instruction to the program.
	 * @param lit The source literal to push.
	 */
	void
	push_16(uint16_t lit)
	{
		push_instruction(PUSH_16);
		push(lit);
	}

	/**
	 * @brief Adds a PUSH_32 instruction to the program.
	 * @param lit The source literal to push.
	 */
	void
	push_32(uint32_t lit)
	{
		push_instruction(PUSH_32);
		push(lit);
	}

	/**
	 * @brief Adds a PUSH_64 instruction to the program.
	 * @param lit The source literal to push.
	 */
	void
	push_64(uint64_t lit)
	{
		push_instruction(PUSH_64);
		push(lit);
	}

	/**
	 * @brief Adds a PUSH_REG_8 instruction to the program.
	 * @param reg_id The source register to push.
	 */
	void
	push_reg_8(uint8_t reg_id)
	{
		push_instruction(PUSH_REG_8);
		push(reg_id);
	}

	/**
	 * @brief Adds a PUSH_REG_16 instruction to the program.
	 * @param reg_id The source register to push.
	 */
	void
	push_reg_16(uint8_t reg_id)
	{
		push_instruction(PUSH_REG_16);
		push(reg_id);
	}

	/**
	 * @brief Adds a PUSH_REG_32 instruction to the program.
	 * @param reg_id The source register to push.
	 */
	void
	push_reg_32(uint8_t reg_id)
	{
		push_instruction(PUSH_REG_32);
		push(reg_id);
	}

	/**
	 * @brief Adds a PUSH_REG_64 instruction to the program.
	 * @param reg_id The source register to push.
	 */
	void
	push_reg_64(uint8_t reg_id)
	{
		push_instruction(PUSH_REG_64);
		push(reg_id);
	}

	/**
	 * @brief Adds a POP_8_INTO_REG instruction to the program.
	 * @param reg_id The destination register to pop into.
	 */
	void
	pop_8_into_reg(uint8_t reg_id)
	{
		push_instruction(POP_8_INTO_REG);
		push(reg_id);
	}

	/**
	 * @brief Adds a POP_16_INTO_REG instruction to the program.
	 * @param reg_id The destination register to pop into.
	 */
	void
	pop_16_into_reg(uint8_t reg_id)
	{
		push_instruction(POP_16_INTO_REG);
		push(reg_id);
	}

	/**
	 * @brief Adds a POP_32_INTO_REG instruction to the program.
	 * @param reg_id The destination register to pop into.
	 */
	void
	pop_32_into_reg(uint8_t reg_id)
	{
		push_instruction(POP_32_INTO_REG);
		push(reg_id);
	}

	/**
	 * @brief Adds a POP_64_INTO_REG instruction to the program.
	 * @param reg_id The destination register to pop into.
	 */
	void
	pop_64_into_reg(uint8_t reg_id)
	{
		push_instruction(POP_64_INTO_REG);
		push(reg_id);
	}

	/**
	 * @brief Adds a CALL instruction to the program.
	 * @param label The label to call.
	 */
	void
	call(const std::string &label)
	{
		push_instruction(CALL);
		add_label_reference(label);
		push<uint64_t>(0); // This will be updated later
	}

	/**
	 * @brief Adds a RETURN instruction to the program.
	 */
	void
	return_()
	{
		push_instruction(RETURN);
	}

	/**
	 * @brief Adds a ALLOCATE_STACK instruction to the program.
	 * @param size The number of bytes to allocate.
	 */
	void
	allocate_stack(uint64_t size)
	{
		push_instruction(ALLOCATE_STACK);
		push(size);
	}

	/**
	 * @brief Adds a DEALLOCATE_STACK instruction to the program.
	 * @param size The number of bytes to deallocate.
	 */
	void
	deallocate_stack(uint64_t size)
	{
		push_instruction(DEALLOCATE_STACK);
		push(size);
	}

	/**
	 * @brief Adds a COMMENT instruction to the program.
	 * @param str The comment content.
	 */
	void
	comment(const std::string &str)
	{
		push_instruction(COMMENT);
		push_null_terminated_string(str);
	}

	/**
	 * @brief Adds a LABEL instruction to the program.
	 * @param str The label name.
	 */
	void
	label(const std::string &str)
	{
		push_instruction(LABEL);
		push_null_terminated_string(str);
	}

	/**
	 * @brief Adds a PRINT_CHAR instruction to the program.
	 * @param reg_id The source register containing the character.
	 */
	void
	print_char(uint8_t reg_id)
	{
		push_instruction(PRINT_CHAR);
		push(reg_id);
	}

	/**
	 * @brief Adds a GET_CHAR instruction to the program.
	 * @param reg_id The destination register for the character.
	 */
	void
	get_char(uint8_t reg_id)
	{
		push_instruction(GET_CHAR);
		push(reg_id);
	}

	/**
	 * @brief Adds a label to the program.
	 * The label can later be referred to using the
	 * `add_label_reference()` method.
	 * @param id The label name.
	 */
	void
	add_label(const std::string &id)
	{
		if (labels.count(id))
		{
			printf("ProgramBuilder error: duplicate label %s\n", id.c_str());
			abort();
		}

		labels[id] = offset;
	}

	/**
	 * @brief Adds a reference to a label to the program.
	 * The label must have been previously added using the
	 * `add_label()` method.
	 * @param id The label name.
	 */
	void
	add_label_reference(const std::string &id)
	{
		label_references[id].push_back(offset);
	}

	/**
	 * @brief Computes the offset to all label references.
	 */
	void
	update_label_references()
	{
		for (std::pair<const std::string &, std::vector<uint64_t>> ref : label_references)
		{
			const std::string &label                = ref.first;
			std::vector<uint64_t> &reference_points = ref.second;

			// Get the location of the label.

			if (!labels.count(label))
			{
				printf("ProgramBuilder error: referenced non-defined label %s\n",
					label.c_str());
				abort();
			}

			uint64_t label_location = labels[label];

			// Update all label references.
			// Label references are relative to the location of the label.

			for (size_t j = 0; j < reference_points.size(); j++)
			{
				uint64_t reference_location         = reference_points[j];
				int64_t relative_reference_location = label_location - reference_location + 2;
				int64_t *reference_point            = (int64_t *) (data() + reference_points[j]);

				*reference_point = relative_reference_location;
			}
		}
	}

	/**
	 * @brief Adds a chunk of static data to the program.
	 * @param data A pointer to the data. The data is copied.
	 * @param size The size of the data.
	 * @returns A StaticData structure containing the location of
	 * the static data entry.
	 */
	struct StaticData
	add_static_data(const uint8_t *data, size_t size)
	{
		size_t offset = static_data.offset;

		for (size_t i = 0; i < size; i++)
		{
			static_data.push(data[i]);
		}

		return {
			.offset = offset,
			.size   = size
		};
	}
};

#endif