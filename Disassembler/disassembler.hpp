#ifndef TEA_DISASSEMBLER_HEADER
#define TEA_DISASSEMBLER_HEADER

#include "file-reader.hpp"
#include "../ansi.hpp"
#include "../Assembler/byte_code.hpp"
#include "../VM/cpu.hpp"

/**
 * @brief Class that is responsible for disassembling a bytecode file.
 */
struct Disassembler
{
	// The file reader for the bytecode file.
	FileReader file_reader;

	// The output file.
	FILE *file_out;

	// Flag that indicates whether an argument is the first
	// or not. Used to determine whether to print a comma or not.
	bool first_arg;

	// The address of the current instruction being disassembled.
	// Used to calculate the absolute address of relative addresses.
	uint64_t instr_addr;

	/**
	 * @brief Constructs a new Disassembler object.
	 * Initialises the file reader and the output file.
	 * @param file_in The input file.
	 * @param file_out The output file.
	 */
	Disassembler(FILE *file_in, FILE *file_out)
		: file_reader(file_in), file_out(file_out), first_arg(true) {}

	/**
	 * @brief Pretty-prints an instruction to the output file.
	 * @param instruction The instruction to be printed.
	 */
	void
	print_instruction(const char *instruction,
		const std::vector<ArgumentType> &args)
	{
		// Print address in green.

		fprintf(file_out,
			ANSI_GREEN "0x" ANSI_BRIGHT_GREEN "%04llx" ANSI_RESET "    ",
			instr_addr);

		// Print instruction in orange.

		fprintf(file_out, ANSI_YELLOW "%-33s" ANSI_RESET, instruction);

		// Print arguments.

		for (ArgumentType arg : args)
		{
			print_arg(arg);
		}

		end_args();
	}

	/**
	 * @brief Pretty-prints an argument to the output file.
	 * @param arg The argument to be printed.
	 */
	void
	print_arg(ArgumentType arg)
	{
		if (first_arg)
		{
			first_arg = false;
			fprintf(file_out, " ");
		}
		else
		{
			fprintf(file_out, ", ");
		}

		switch (arg)
		{
		case REG:
			print_arg_reg(file_reader.read<uint8_t>());
			break;

		case REL_ADDR:
			print_arg_rel_address(file_reader.read<int64_t>());
			break;

		case ABS_ADDR:
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
			std::vector<char> str;
			char c;

			do
			{
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

	/**
	 * @brief Pretty-prints a register argument to the output file.
	 * @param reg_id The register id.
	 */
	void
	print_arg_reg(uint8_t reg_id)
	{
		fprintf(file_out, ANSI_CYAN "%s" ANSI_RESET, CPU::reg_to_str(reg_id));
	}

	/**
	 * @brief Pretty-prints a literal argument to the output file.
	 * @param num The literal number to print.
	 */
	void
	print_arg_literal_number(uint64_t num)
	{
		fprintf(file_out, ANSI_YELLOW "%lld" ANSI_RESET, num);
	}

	/**
	 * @brief Pretty-prints a relative address argument to the
	 * output file.
	 * @param str The relative address to print.
	 */
	void
	print_arg_rel_address(int64_t rel_address)
	{
		uint64_t addr = instr_addr + rel_address;
		fprintf(file_out, ANSI_YELLOW "%lld " ANSI_RESET, rel_address);
		fprintf(file_out,
			"(" ANSI_GREEN "0x" ANSI_BRIGHT_GREEN "%llx" ANSI_RESET ")",
			addr);
	}

	/**
	 * @brief Pretty-prints an address argument to the output file.
	 * @param str The address to print.
	 */
	void
	print_arg_address(uint64_t address)
	{
		fprintf(file_out,
			ANSI_GREEN "0x" ANSI_BRIGHT_GREEN "%llx" ANSI_RESET,
			address);
	}

	/**
	 * @brief Prints a string comment argument to the output file.
	 * @param str The string to print.
	 */
	void
	print_arg_null_terminated_string(char *str)
	{
		fprintf(file_out, ANSI_BRIGHT_BLACK "/* %s */", str);
	}

	/**
	 * @brief Ends the current instruction.
	 */
	void
	end_args()
	{
		fprintf(file_out, "\n");
		first_arg = true;
	}

	/**
	 * @brief Disassembles the bytecode file.
	 * The instructions are printed to the output file.
	 */
	void
	disassemble()
	{
		uint64_t static_data_size = file_reader.read<uint64_t>();
		uint64_t program_size     = file_reader.read<uint64_t>();

		// Print static data.

		fprintf(file_out, "Static data (size = %llu)\n\n", static_data_size);

		for (size_t i = 0; i < static_data_size; i++)
		{
			uint8_t byte = file_reader.read<uint8_t>();

			fprintf(file_out, "0x%04lx    0x%02hhx    %03hhu    '%c'\n",
				i, byte, byte, byte);
		}

		// Print the program.

		fprintf(file_out, "\nProgram (size = %llu)\n\n", program_size);

		Instruction instruction;

		while (
			(instruction = (Instruction) file_reader.read<uint16_t>())
			!= (1 << 16) - 1)
		{
			const char *instruction_str    = instruction_to_str(instruction);
			std::vector<ArgumentType> args = instruction_arg_types(instruction);

			instr_addr = file_reader.read_bytes - 18;
			print_instruction(instruction_str, args);
		}
	}
};

#endif