#ifndef TEA_ASSEMBLER_HEADER
#define TEA_ASSEMBLER_HEADER

#include <unordered_map>
#include <stack>

#include "VM/cpu.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/code-gen/buffer-builder.hpp"

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
	int64_t offset;

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

	// Generator for unique label identifiers.
	// Used for generating unique labels for different
	// code segments. Starts at 0.
	uint64_t label_id = 0;

	// A stack containing the current loop labels.
	// Used for the break and continue statements.
	std::stack<std::pair<std::string, std::string>> loop_labels;

	/**
	 * @brief Bit array used to check whether a register is free.
	 */
	std::vector<bool> free_registers;

	/**
	 * @brief A buffer builder for the static data segment.
	 * It is later built and prepended to the program.
	 */
	BufferBuilder static_data;

	// Whether debug symbols should be generated.
	const bool debug;

	/**
	 * @brief Construct a new CodeGenState object.
	 */
	Assembler(bool debug)
		: free_registers(GENERAL_PURPOSE_REGISTER_COUNT, true),
		  debug(debug) {}

	/**
	 * @brief Destroy the CodeGenState object.
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

		for (uint8_t i = 0; i < GENERAL_PURPOSE_REGISTER_COUNT; i++)
		{
			if (free_registers[i])
			{
				free_registers[i] = false;
				return i;
			}
		}

		// Todo: create

		p_warn(stderr, "No free registers found. Spilling registers is not implemented yet.");
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

		for (ssize_t j = static_data.offset - 1; j >= 0; j--)
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
	push_instruction(Instruction instruction)
	{
		push<uint16_t>(instruction);
	}

	/**
	 * @brief Adds a MOVE_LIT instruction to the program.
	 * @param lit The source literal.
	 * @param reg_id The destination register.
	 */
	void
	move_lit(uint64_t lit, uint8_t reg_id)
	{
		push_instruction(MOVE_LIT);
		push(lit);
		push(reg_id);
	}

	/**
	 * @brief Adds a MOVE instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	move(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(MOVE);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a LOAD_PTR_8 instruction to the program.
	 * @param reg_id_1 The source register that holds a pointer.
	 * @param reg_id_2 The destination register.
	 */
	void
	load_ptr_8(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(LOAD_PTR_8);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a LOAD_PTR_16 instruction to the program.
	 * @param reg_id_1 The source register that holds a pointer.
	 * @param reg_id_2 The destination register.
	 */
	void
	load_ptr_16(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(LOAD_PTR_16);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a LOAD_PTR_32 instruction to the program.
	 * @param reg_id_1 The source register that holds a pointer.
	 * @param reg_id_2 The destination register.
	 */
	void
	load_ptr_32(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(LOAD_PTR_32);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a LOAD_PTR_64 instruction to the program.
	 * @param reg_id_1 The source register that holds a pointer.
	 * @param reg_id_2 The destination register.
	 */
	void
	load_ptr_64(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(LOAD_PTR_64);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a STORE_PTR_8 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register that holds a pointer.
	 */
	void
	store_ptr_8(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(STORE_PTR_8);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a STORE_PTR_16 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register that holds a pointer.
	 */
	void
	store_ptr_16(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(STORE_PTR_16);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a STORE_PTR_32 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register that holds a pointer.
	 */
	void
	store_ptr_32(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(STORE_PTR_32);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a STORE_PTR_64 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register that holds a pointer.
	 */
	void
	store_ptr_64(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(STORE_PTR_64);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a MEM_COPY instruction to the program.
	 * @param reg_id_src The source register that holds a pointer.
	 * @param reg_id_dst The destination register that holds a pointer.
	 * @param size The number of bytes to copy.
	 */
	void
	mem_copy(uint8_t reg_id_src, uint8_t reg_id_dst, uint64_t size)
	{
		push_instruction(MEM_COPY);
		push(reg_id_src);
		push(reg_id_dst);
		push(size);
	}

	/**
	 * @brief Adds a ADD_INT_8 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	add_int_8(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(ADD_INT_8);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a ADD_INT_16 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	add_int_16(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(ADD_INT_16);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a ADD_INT_32 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	add_int_32(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(ADD_INT_32);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a ADD_INT_64 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	add_int_64(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(ADD_INT_64);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a ADD_FLT_32 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	add_flt_32(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(ADD_FLT_32);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a ADD_FLT_64 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	add_flt_64(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(ADD_FLT_64);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a SUB_INT_8 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	sub_int_8(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(SUB_INT_8);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a SUB_INT_16 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	sub_int_16(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(SUB_INT_16);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a SUB_INT_32 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	sub_int_32(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(SUB_INT_32);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a SUB_INT_64 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	sub_int_64(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(SUB_INT_64);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a SUB_FLT_32 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	sub_flt_32(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(SUB_FLT_32);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a SUB_FLT_64 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	sub_flt_64(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(SUB_FLT_64);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a MUL_INT_8 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	mul_int_8(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(MUL_INT_8);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a MUL_INT_16 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	mul_int_16(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(MUL_INT_16);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a MUL_INT_32 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	mul_int_32(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(MUL_INT_32);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a MUL_INT_64 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	mul_int_64(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(MUL_INT_64);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a MUL_FLT_32 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	mul_flt_32(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(MUL_FLT_32);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a MUL_FLT_64 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	mul_flt_64(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(MUL_FLT_64);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a DIV_INT_8 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	div_int_8(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(DIV_INT_8);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a DIV_INT_16 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	div_int_16(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(DIV_INT_16);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a DIV_INT_32 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	div_int_32(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(DIV_INT_32);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a DIV_INT_64 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	div_int_64(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(DIV_INT_64);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a DIV_FLT_32 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	div_flt_32(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(DIV_FLT_32);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a DIV_FLT_64 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	div_flt_64(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(DIV_FLT_64);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a MOD_INT_8 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	mod_int_8(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(MOD_INT_8);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a MOD_INT_16 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	mod_int_16(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(MOD_INT_16);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a MOD_INT_32 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	mod_int_32(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(MOD_INT_32);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a MOD_INT_64 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	mod_int_64(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(MOD_INT_64);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a AND_INT_8 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	and_int_8(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(AND_INT_8);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a AND_INT_16 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	and_int_16(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(AND_INT_16);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a AND_INT_32 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	and_int_32(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(AND_INT_32);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a AND_INT_64 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	and_int_64(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(AND_INT_64);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a OR_INT_8 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	or_int_8(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(OR_INT_8);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a OR_INT_16 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	or_int_16(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(OR_INT_16);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a OR_INT_32 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	or_int_32(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(OR_INT_32);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a OR_INT_64 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	or_int_64(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(OR_INT_64);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a XOR_INT_8 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	xor_int_8(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(XOR_INT_8);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a XOR_INT_16 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	xor_int_16(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(XOR_INT_16);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a XOR_INT_32 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	xor_int_32(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(XOR_INT_32);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a XOR_INT_64 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */

	void
	xor_int_64(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(XOR_INT_64);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a SHL_INT_8 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	shl_int_8(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(SHL_INT_8);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a SHL_INT_16 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	shl_int_16(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(SHL_INT_16);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a SHL_INT_32 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	shl_int_32(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(SHL_INT_32);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a SHL_INT_64 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	shl_int_64(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(SHL_INT_64);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a SHR_INT_8 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	shr_int_8(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(SHR_INT_8);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a SHR_INT_16 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	shr_int_16(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(SHR_INT_16);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a SHR_INT_32 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	shr_int_32(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(SHR_INT_32);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a SHR_INT_64 instruction to the program.
	 * @param reg_id_1 The source register.
	 * @param reg_id_2 The destination register.
	 */
	void
	shr_int_64(uint8_t reg_id_1, uint8_t reg_id_2)
	{
		push_instruction(SHR_INT_64);
		push(reg_id_1);
		push(reg_id_2);
	}

	/**
	 * @brief Adds a INC_INT_8 instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	inc_int_8(uint8_t reg_id)
	{
		push_instruction(INC_INT_8);
		push(reg_id);
	}

	/**
	 * @brief Adds a INC_INT_16 instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	inc_int_16(uint8_t reg_id)
	{
		push_instruction(INC_INT_16);
		push(reg_id);
	}

	/**
	 * @brief Adds a INC_INT_32 instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	inc_int_32(uint8_t reg_id)
	{
		push_instruction(INC_INT_32);
		push(reg_id);
	}

	/**
	 * @brief Adds a INC_INT_64 instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	inc_int_64(uint8_t reg_id)
	{
		push_instruction(INC_INT_64);
		push(reg_id);
	}

	/**
	 * @brief Adds a DEC_INT_8 instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	dec_int_8(uint8_t reg_id)
	{
		push_instruction(DEC_INT_8);
		push(reg_id);
	}

	/**
	 * @brief Adds a DEC_INT_16 instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	dec_int_16(uint8_t reg_id)
	{
		push_instruction(DEC_INT_16);
		push(reg_id);
	}

	/**
	 * @brief Adds a DEC_INT_32 instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	dec_int_32(uint8_t reg_id)
	{
		push_instruction(DEC_INT_32);
		push(reg_id);
	}

	/**
	 * @brief Adds a DEC_INT_64 instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	dec_int_64(uint8_t reg_id)
	{
		push_instruction(DEC_INT_64);
		push(reg_id);
	}

	/**
	 * @brief Adds a NEG_INT_8 instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	neg_int_8(uint8_t reg_id)
	{
		push_instruction(NEG_INT_8);
		push(reg_id);
	}

	/**
	 * @brief Adds a NEG_INT_16 instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	neg_int_16(uint8_t reg_id)
	{
		push_instruction(NEG_INT_16);
		push(reg_id);
	}

	/**
	 * @brief Adds a NEG_INT_32 instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	neg_int_32(uint8_t reg_id)
	{
		push_instruction(NEG_INT_32);
		push(reg_id);
	}

	/**
	 * @brief Adds a NEG_INT_64 instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	neg_int_64(uint8_t reg_id)
	{
		push_instruction(NEG_INT_64);
		push(reg_id);
	}

	/**
	 * @brief Adds a CAST_INT_TO_FLT_32 instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	cast_int_to_flt_32(uint8_t reg_id)
	{
		push_instruction(CAST_INT_TO_FLT_32);
		push(reg_id);
	}

	/**
	 * @brief Adds a CAST_INT_TO_FLT_64 instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	cast_int_to_flt_64(uint8_t reg_id)
	{
		push_instruction(CAST_INT_TO_FLT_64);
		push(reg_id);
	}

	/**
	 * @brief Adds a CAST_FLT_32_TO_INT instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	cast_flt_32_to_int(uint8_t reg_id)
	{
		push_instruction(CAST_FLT_32_TO_INT);
		push(reg_id);
	}

	/**
	 * @brief Adds a CAST_FLT_64_TO_INT instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	cast_flt_64_to_int(uint8_t reg_id)
	{
		push_instruction(CAST_FLT_64_TO_INT);
		push(reg_id);
	}

	/**
	 * @brief Adds a CMP_INT_8 instruction to the program.
	 * @param reg_id_1 The register with the compared value.
	 * @param reg_id_2 The register to compare against.
	 */
	void
	cmp_int_8(uint8_t reg_id_1, uint8_t reg_id)
	{
		push_instruction(CMP_INT_8);
		push(reg_id_1);
		push(reg_id);
	}

	/**
	 * @brief Adds a CMP_INT_8_U instruction to the program.
	 * @param reg_id_1 The register with the compared value.
	 * @param reg_id_2 The register to compare against.
	 */
	void
	cmp_int_8_u(uint8_t reg_id_1, uint8_t reg_id)
	{
		push_instruction(CMP_INT_8_U);
		push(reg_id_1);
		push(reg_id);
	}

	/**
	 * @brief Adds a CMP_INT_16 instruction to the program.
	 * @param reg_id_1 The register with the compared value.
	 * @param reg_id_2 The register to compare against.
	 */
	void
	cmp_int_16(uint8_t reg_id_1, uint8_t reg_id)
	{
		push_instruction(CMP_INT_16);
		push(reg_id_1);
		push(reg_id);
	}

	/**
	 * @brief Adds a CMP_INT_16_U instruction to the program.
	 * @param reg_id_1 The register with the compared value.
	 * @param reg_id_2 The register to compare against.
	 */
	void
	cmp_int_16_u(uint8_t reg_id_1, uint8_t reg_id)
	{
		push_instruction(CMP_INT_16_U);
		push(reg_id_1);
		push(reg_id);
	}

	/**
	 * @brief Adds a CMP_INT_32 instruction to the program.
	 * @param reg_id_1 The register with the compared value.
	 * @param reg_id_2 The register to compare against.
	 */
	void
	cmp_int_32(uint8_t reg_id_1, uint8_t reg_id)
	{
		push_instruction(CMP_INT_32);
		push(reg_id_1);
		push(reg_id);
	}

	/**
	 * @brief Adds a CMP_INT_32_U instruction to the program.
	 * @param reg_id_1 The register with the compared value.
	 * @param reg_id_2 The register to compare against.
	 */
	void
	cmp_int_32_u(uint8_t reg_id_1, uint8_t reg_id)
	{
		push_instruction(CMP_INT_32_U);
		push(reg_id_1);
		push(reg_id);
	}

	/**
	 * @brief Adds a CMP_INT_64 instruction to the program.
	 * @param reg_id_1 The register with the compared value.
	 * @param reg_id_2 The register to compare against.
	 */
	void
	cmp_int_64(uint8_t reg_id_1, uint8_t reg_id)
	{
		push_instruction(CMP_INT_64);
		push(reg_id_1);
		push(reg_id);
	}

	/**
	 * @brief Adds a CMP_INT_64_U instruction to the program.
	 * @param reg_id_1 The register with the compared value.
	 * @param reg_id_2 The register to compare against.
	 */
	void
	cmp_int_64_u(uint8_t reg_id_1, uint8_t reg_id)
	{
		push_instruction(CMP_INT_64_U);
		push(reg_id_1);
		push(reg_id);
	}

	/**
	 * @brief Adds a CMP_FLT_32 instruction to the program.
	 * @param reg_id_1 The register with the compared value.
	 * @param reg_id_2 The register to compare against.
	 */
	void
	cmp_flt_32(uint8_t reg_id_1, uint8_t reg_id)
	{
		push_instruction(CMP_FLT_32);
		push(reg_id_1);
		push(reg_id);
	}

	/**
	 * @brief Adds a CMP_FLT_64 instruction to the program.
	 * @param reg_id_1 The register with the compared value.
	 * @param reg_id_2 The register to compare against.
	 */
	void
	cmp_flt_64(uint8_t reg_id_1, uint8_t reg_id)
	{
		push_instruction(CMP_FLT_64);
		push(reg_id_1);
		push(reg_id);
	}

	/**
	 * @brief Adds a SET_IF_GT instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	set_if_gt(uint8_t reg_id)
	{
		push_instruction(SET_IF_GT);
		push(reg_id);
	}

	/**
	 * @brief Adds a SET_IF_GEQ instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	set_if_geq(uint8_t reg_id)
	{
		push_instruction(SET_IF_GEQ);
		push(reg_id);
	}

	/**
	 * @brief Adds a SET_IF_LT instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	set_if_lt(uint8_t reg_id)
	{
		push_instruction(SET_IF_LT);
		push(reg_id);
	}

	/**
	 * @brief Adds a SET_IF_LEQ instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	set_if_leq(uint8_t reg_id)
	{
		push_instruction(SET_IF_LEQ);
		push(reg_id);
	}

	/**
	 * @brief Adds a SET_IF_EQ instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	set_if_eq(uint8_t reg_id)
	{
		push_instruction(SET_IF_EQ);
		push(reg_id);
	}

	/**
	 * @brief Adds a SET_IF_NEQ instruction to the program.
	 * @param reg_id The destination register.
	 */
	void
	set_if_neq(uint8_t reg_id)
	{
		push_instruction(SET_IF_NEQ);
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
	 * @brief Adds a JUMP_IF_GT instruction to the program.
	 * @param label The label to jump to.
	 */
	void
	jump_if_gt(const std::string &label)
	{
		push_instruction(JUMP_IF_GT);
		add_label_reference(label);
		push<uint64_t>(0); // This will be updated later
	}

	/**
	 * @brief Adds a JUMP_IF_GEQ instruction to the program.
	 * @param label The label to jump to.
	 */
	void
	jump_if_geq(const std::string &label)
	{
		push_instruction(JUMP_IF_GEQ);
		add_label_reference(label);
		push<uint64_t>(0); // This will be updated later
	}

	/**
	 * @brief Adds a JUMP_IF_LT instruction to the program.
	 * @param label The label to jump to.
	 */
	void
	jump_if_lt(const std::string &label)
	{
		push_instruction(JUMP_IF_LT);
		add_label_reference(label);
		push<uint64_t>(0); // This will be updated later
	}

	/**
	 * @brief Adds a JUMP_IF_LEQ instruction to the program.
	 * @param label The label to jump to.
	 */
	void
	jump_if_leq(const std::string &label)
	{
		push_instruction(JUMP_IF_LEQ);
		add_label_reference(label);
		push<uint64_t>(0); // This will be updated later
	}

	/**
	 * @brief Adds a JUMP_IF_EQ instruction to the program.
	 * @param label The label to jump to.
	 */
	void
	jump_if_eq(const std::string &label)
	{
		push_instruction(JUMP_IF_EQ);
		add_label_reference(label);
		push<uint64_t>(0); // This will be updated later
	}

	/**
	 * @brief Adds a JUMP_IF_NEQ instruction to the program.
	 * @param label The label to jump to.
	 */
	void
	jump_if_neq(const std::string &label)
	{
		push_instruction(JUMP_IF_NEQ);
		add_label_reference(label);
		push<uint64_t>(0); // This will be updated later
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
			p_warn(stderr, "ProgramBuilder error: duplicate label %s\n", id.c_str());
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
				p_warn(stderr, "ProgramBuilder error: referenced non-defined label %s\n",
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
	 * @brief Generates a label name that can be used to jump to.
	 * @param type The type of label to generate.
	 * @returns The generated label name.
	 */
	std::string
	generate_label(std::string type)
	{
		std::string label = "compiler-generated-label-";
		label += std::to_string(label_id++);
		label += "-for-";
		label += type;
		return label;
	}

	/**
	 * @brief Starts a new scope within the current function being compiled.
	 * @returns The start and end labels of the new scope.
	 */
	std::pair<std::string, std::string>
	push_loop_scope()
	{
		std::pair<std::string, std::string> labels = std::make_pair<>(
			generate_label("loop-start"), generate_label("loop-end"));
		loop_labels.push(labels);
		return labels;
	}

	/**
	 * @brief Ends the current scope within the current function being compiled.
	 */
	void
	pop_loop_scope()
	{
		loop_labels.pop();
	}

	/**
	 * @brief Adds a chunk of static data to the program.
	 * @param data A pointer to the data. The data is copied.
	 * @param size The size of the data.
	 * @returns A StaticData structure containing the location of
	 * the static data entry.
	 */
	StaticData
	add_static_data(const std::string &data)
	{
		ssize_t offset = -(static_data.offset + data.size() + 1);

		// Data is written in reverse, since in code it should be referenced
		// with a negative offset from the stack top. This way we can compute
		// the offset immediately, without having to know the size of the data.

		static_data.push('\0');

		for (ssize_t i = data.size() - 1; i >= 0; i--)
		{
			static_data.push(data[i]);
		}

		return {
			.offset = offset,
			.size   = data.size(),
		};
	}
};

#endif