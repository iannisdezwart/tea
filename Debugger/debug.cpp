#include <bits/stdc++.h>
#include <sys/stat.h>

#define RESTORE_INSTRUCTION_POINTER_ON_THROW

#include "../ansi.hpp"
#include "../VM/cpu.hpp"
#include "../VM/memory.hpp"
#include "../Assembler/assembler.hpp"
#include "util.hpp"
#include "command.hpp"
#include "instruction-lister.hpp"
#include "../Compiler/debugger-symbols.hpp"

#define DEFAULT_STACK_SIZE     2048
#define DEFAULT_LIST_TOP       15
#define DEFAULT_CALL_STACK_TOP 10
#define DEFAULT_DUMP_STACK_TOP 24

#define PROGRAM_NOT_STARTED_ERR()                                                               \
	printf(ANSI_RED "Program has not yet started.\n"                                        \
			"Start the program with the " ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD \
			"start" ANSI_RESET ANSI_RED " command.\n")

struct CallStackEntryArg
{
	std::string arg_name;
	std::string value;
};

struct CallStackEntry
{
	std::string fn_name;
	std::vector<CallStackEntryArg> args;
};

struct VarEntry : public DebuggerSymbol
{
	uint8_t *addr;

	VarEntry(const DebuggerSymbol &sym, uint8_t *addr)
		: DebuggerSymbol(sym), addr(addr) {}
};

struct Shell
{
	const char *file_path;
	CPU *cpu = NULL;
	PtrSet breakpoints;
	std::vector<CallStackEntry> call_stack;
	std::vector<std::vector<VarEntry>> locals;
	std::vector<VarEntry> globals;
	DebuggerSymbols debugger_symbols;
	bool debugger_symbols_found = false;

	void
	collect_fn_call_arg_details(CallStackEntry &entry,
		const DebuggerSymbol &param, size_t &params_offset)
	{
		CallStackEntryArg entry_arg;
		entry_arg.arg_name = param.name;
		uint8_t *addr      = cpu->get_frame_ptr() - params_offset - 8 - CPU::stack_frame_size;

		switch (param.type)
		{
		case DebuggerSymbolTypes::POINTER:
		{
			uint64_t val    = memory::get<uint64_t>(addr);
			entry_arg.value = ANSI_GREEN "0x" ANSI_BRIGHT_GREEN + to_hex_str(addr);
			break;
		}

		case DebuggerSymbolTypes::U8:
		{
			uint8_t val     = memory::get<uint8_t>(addr);
			entry_arg.value = ANSI_YELLOW + std::to_string(val);
			break;
		}

		case DebuggerSymbolTypes::I8:
		{
			int8_t val      = memory::get<int8_t>(addr);
			entry_arg.value = ANSI_YELLOW + std::to_string(val);
			break;
		}

		case DebuggerSymbolTypes::U16:
		{
			uint16_t val    = memory::get<uint16_t>(addr);
			entry_arg.value = ANSI_YELLOW + std::to_string(val);
			break;
		}

		case DebuggerSymbolTypes::I16:
		{
			int16_t val     = memory::get<int16_t>(addr);
			entry_arg.value = ANSI_YELLOW + std::to_string(val);
			break;
		}

		case DebuggerSymbolTypes::U32:
		{
			uint32_t val    = memory::get<uint32_t>(addr);
			entry_arg.value = ANSI_YELLOW + std::to_string(val);
			break;
		}

		case DebuggerSymbolTypes::I32:
		{
			int32_t val     = memory::get<int32_t>(addr);
			entry_arg.value = ANSI_YELLOW + std::to_string(val);
			break;
		}

		case DebuggerSymbolTypes::I64:
		{
			int64_t val     = memory::get<int64_t>(addr);
			entry_arg.value = ANSI_YELLOW + std::to_string(val);
			break;
		}

		case DebuggerSymbolTypes::U64:
		{
			uint64_t val    = memory::get<uint64_t>(addr);
			entry_arg.value = ANSI_YELLOW + std::to_string(val);
			break;
		}

		default:
		{
			fprintf(stderr, "Unknown parameter type: %hhu\n", param.type);
			abort();
		}
		}

		params_offset -= param.byte_size();
		entry.args.push_back(entry_arg);
	}

	void
	collect_fn_call_details(CallStackEntry &entry)
	{
		const DebuggerFunction &fn_symbols = debugger_symbols.functions[entry.fn_name];
		size_t params_offset               = 0;

		// Calculate the size of the parameters.

		for (const DebuggerSymbol &param : fn_symbols.params)
		{
			params_offset += param.byte_size();
		}

		// Store the argument values.

		for (const DebuggerSymbol &param : fn_symbols.params)
		{
			collect_fn_call_arg_details(entry, param, params_offset);
		}

		// Store addresses of locals.

		std::vector<VarEntry> new_fn_locals;
		uint8_t *addr = cpu->get_stack_ptr();

		for (const DebuggerSymbol &local : fn_symbols.locals)
		{
			new_fn_locals.push_back(VarEntry(local, addr));
			addr += local.byte_size();
		}

		locals.push_back(new_fn_locals);
	}

	void
	exec_instruction()
	{
		// Get the next instruction.

		Instruction instruction = cpu->step();

		switch (instruction)
		{
		case CALL:
		{
			// Add this function call to call stack.

			Instruction next_instruction = (Instruction) memory::get<uint16_t>(
				cpu->get_instr_ptr());

			std::stringstream ss;

			// If the function has a debug label, push its name.

			if (next_instruction == LABEL)
			{
				uint8_t *label_offset = cpu->get_instr_ptr() + 2;
				char c;

				while (true)
				{
					c = memory::get<char>(label_offset++);
					if (c == '\0')
						break;
					ss << c;
				}
			}

			// Else, push its address.

			else
			{
				ss << "0x" << std::hex << cpu->get_instr_ptr();
			}

			// Create a call stack entry.

			CallStackEntry entry;
			entry.fn_name = ss.str();

			// Check if there are debugger symbols.

			if (debugger_symbols.functions.count(entry.fn_name))
			{
				collect_fn_call_details(entry);
			}

			call_stack.push_back(entry);
			break;
		}

		case RETURN:
		{
			// Pop from the call stack and the locals.

			call_stack.pop_back();
			locals.pop_back();
		}

		default:;
		}
	}

	void
	step()
	{
		// Execute the next instruction, if the program is still running.

		if (cpu->get_instr_ptr() < cpu->stack_top)
		{
			try
			{
				exec_instruction();
			}
			catch (const std::string &err_message)
			{
				printf(ANSI_RED "The VM encountered an error:\n" ANSI_BRIGHT_RED "%s",
					err_message.c_str());
			}
		}
		else
		{
			printf(ANSI_RED "Program ended, cannot step\n");
		}
	}

	void
	run()
	{
		// Execute the next instruction.

		if (cpu->get_instr_ptr() < cpu->stack_top)
		{
			try
			{
				exec_instruction();
			}
			catch (const std::string &err_message)
			{
				printf(ANSI_RED "The VM encountered an error:\n" ANSI_BRIGHT_RED "%s",
					err_message.c_str());
				return;
			}
		}

		// And keep executing until a breakpoint is hit, or the program finishes.

		while (cpu->get_instr_ptr() < cpu->stack_top)
		{
			if (breakpoints.count(cpu->get_instr_ptr()))
			{
				return;
			}

			try
			{
				exec_instruction();
			}
			catch (const std::string &err_message)
			{
				printf(ANSI_RED "The VM encountered an error:\n" ANSI_BRIGHT_RED "%s",
					err_message.c_str());
				return;
			}
		}

		// Print the exit code.

		printf(ANSI_CYAN "VM exited with exit code " ANSI_YELLOW "%llu\n",
			cpu->regs[R_RET]);
	}

	Shell(const char *file_path)
		: file_path(file_path) {}

	void
	print_call_stack_entry(const CallStackEntry &entry,
		const std::vector<VarEntry> cur_locals, size_t level)
	{
		// Print function name.

		printf(ANSI_BRIGHT_MAGENTA ANSI_BOLD "  # %lu " ANSI_BRIGHT_BLUE "%s" ANSI_BRIGHT_BLACK "(" ANSI_RESET,
			level, entry.fn_name.c_str());

		// Print args.

		for (size_t i = 0; i < entry.args.size(); i++)
		{
			// Print arg name and value.

			printf(ANSI_BRIGHT_RED ANSI_ITALIC "%s" ANSI_RESET ANSI_BRIGHT_BLACK " = %s" ANSI_RESET,
				entry.args[i].arg_name.c_str(), entry.args[i].value.c_str());

			// Print comma seperator.

			if (i != entry.args.size() - 1)
			{
				printf(ANSI_BRIGHT_BLACK ", ");
			}
		}

		// Print closing parenthesis.

		printf(ANSI_BRIGHT_BLACK ANSI_BOLD ")" ANSI_RESET "\n");

		// Print locals.

		for (size_t i = 0; i < cur_locals.size(); i++)
		{
			const VarEntry &local = cur_locals[i];
			const char *conn_chars;

			if (i == cur_locals.size() - 1)
			{
				conn_chars = "╰─";
			}
			else
			{
				conn_chars = "├─";
			}

#define PRINT_VAR(printf_flag, val) printf(ANSI_BRIGHT_MAGENTA ANSI_BOLD                                     \
	"    %s " ANSI_BLUE "%s" ANSI_RESET ANSI_BRIGHT_BLACK " = " ANSI_YELLOW printf_flag ANSI_RESET "\n", \
	conn_chars, local.name.c_str(), val)

			switch (local.type)
			{
			case DebuggerSymbolTypes::U8:
			{
				uint8_t val = memory::get<uint8_t>(local.addr);
				PRINT_VAR("%hhu", val);
				break;
			}

			case DebuggerSymbolTypes::I8:
			{
				int8_t val = memory::get<int8_t>(local.addr);
				PRINT_VAR("%hhd", val);
				break;
			}

			case DebuggerSymbolTypes::U16:
			{
				uint16_t val = memory::get<uint16_t>(local.addr);
				PRINT_VAR("%hu", val);
				break;
			}

			case DebuggerSymbolTypes::I16:
			{
				int16_t val = memory::get<int16_t>(local.addr);
				PRINT_VAR("%hd", val);
				break;
			}

			case DebuggerSymbolTypes::U32:
			{
				uint32_t val = memory::get<uint32_t>(local.addr);
				PRINT_VAR("%u", val);
				break;
			}

			case DebuggerSymbolTypes::I32:
			{
				int32_t val = memory::get<int32_t>(local.addr);
				PRINT_VAR("%d", val);
				break;
			}

			case DebuggerSymbolTypes::U64:
			{
				uint64_t val = memory::get<uint64_t>(local.addr);
				PRINT_VAR("%llu", val);
				break;
			}

			case DebuggerSymbolTypes::I64:
			{
				int64_t val = memory::get<int64_t>(local.addr);
				PRINT_VAR("%lld", val);
				break;
			}

			case DebuggerSymbolTypes::POINTER:
			{
				int64_t val = memory::get<uint64_t>(local.addr);
				printf(ANSI_BRIGHT_MAGENTA ANSI_BOLD
					"    - " ANSI_BLUE "%s" ANSI_RESET ANSI_BRIGHT_BLACK " = " ANSI_GREEN "0x" ANSI_BRIGHT_GREEN "%s" ANSI_RESET "\n",
					local.name.c_str(), to_hex_str(val).c_str());
				break;
			}

			case DebuggerSymbolTypes::USER_DEFINED_CLASS:
			{
				// Todo: create

				printf(ANSI_BRIGHT_MAGENTA ANSI_BOLD
					"    - " ANSI_BLUE "%s" ANSI_RESET ANSI_BRIGHT_BLACK " = " ANSI_YELLOW "... (not implemented)" ANSI_RESET "\n",
					local.name.c_str());
				break;
			}

			default:
			{
				printf(ANSI_BRIGHT_MAGENTA ANSI_BOLD
					"    - " ANSI_BLUE "???" ANSI_RESET ANSI_BRIGHT_BLACK " = " ANSI_YELLOW "???" ANSI_RESET "\n");
				break;
			}
			}

#undef PRINT_VAR
		}
	}

	void
	run_shell()
	{
	shell:

		// Show the prompt and read the command from the user.

		Command command = Command::read(file_path, cpu == NULL ? NULL : cpu->get_instr_ptr());

		// If the command is empty, try again.

		if (command.num_of_args() == 0)
		{
			goto shell;
		}

		// Load the program.

		if (command[0] == "start")
		{
			std::string stack_size_flag = command.get_flag_value("stack-size").c_str();
			size_t stack_size;

			if (stack_size_flag == "")
			{
				stack_size = DEFAULT_STACK_SIZE;
			}
			else
			{
				stack_size = atoi(stack_size_flag.c_str());
			}

			Executable executable = Executable::from_file(file_path);
			cpu                   = new CPU(executable, stack_size);

			printf(ANSI_BRIGHT_MAGENTA "Loaded executable" ANSI_RESET "\n");

			std::string debug_symbols_filename;
			debug_symbols_filename += file_path;
			debug_symbols_filename += ".debug";

			struct stat file_stats;

			// Read debugger symbols if found.

			if (stat(debug_symbols_filename.c_str(), &file_stats) == 0)
			{
				printf(ANSI_BRIGHT_MAGENTA "Found debugger symbols" ANSI_RESET "\n");
				debugger_symbols       = DebuggerSymbols::parse(debug_symbols_filename);
				debugger_symbols_found = true;
			}
			else
			{
				debugger_symbols_found = false;
			}

			// Store addresses of globals.

			uint8_t *addr = cpu->stack_top;

			for (const DebuggerSymbol &sym : debugger_symbols.globals)
			{
				globals.push_back(VarEntry(sym, addr));
				addr += sym.byte_size();
			}
		}

		// Execute the next instruction of the program.

		else if (command[0] == "step" || command[0] == "s")
		{
			if (cpu == NULL)
			{
				PROGRAM_NOT_STARTED_ERR();
				goto shell;
			}

			step();
		}

		// Execute the program until the next breakpoint is hit, or the program ends.

		else if (command[0] == "run" || command[0] == "r")
		{
			if (cpu == NULL)
			{
				PROGRAM_NOT_STARTED_ERR();
				goto shell;
			}

			run();
		}

		// Pretty print the CPU instructions.

		else if (command[0] == "list" || command[0] == "l")
		{
			if (cpu == NULL)
			{
				PROGRAM_NOT_STARTED_ERR();
				goto shell;
			}

			std::string top_flag = command.get_flag_value("top").c_str();
			size_t top;

			memory::Reader reader(cpu->get_instr_ptr());
			InstructionLister lister(reader);

			// Get the top flag value.

			if (top_flag == "")
			{
				top = DEFAULT_LIST_TOP;
			}
			else if (top_flag == "*")
			{
				lister.disassemble_all(cpu->stack_top, breakpoints);
				goto shell;
			}
			else
			{
				top = atoi(top_flag.c_str());
			}

			// Dissassemble the program.

			lister.disassemble(top, breakpoints);
		}

		// Pretty print the current CPU register states.

		else if (command[0] == "registers" || command[0] == "reg")
		{
			if (cpu == NULL)
			{
				PROGRAM_NOT_STARTED_ERR();
				goto shell;
			}

#define REG_FMT_STR(reg) ANSI_BOLD ANSI_ITALIC reg ANSI_RESET " = " ANSI_GREEN \
							      "0x" ANSI_BRIGHT_GREEN "%016llx" ANSI_RESET " = " ANSI_YELLOW "%020llu" ANSI_RESET "\n"

#define FLAG_FMT_STR(flag) ANSI_BOLD ANSI_ITALIC flag ANSI_RESET " = " ANSI_YELLOW "%hhu" ANSI_RESET "\n"

			printf(REG_FMT_STR(ANSI_BRIGHT_RED "r_instr_ptr"),
				(uint64_t) cpu->get_instr_ptr(), (uint64_t) cpu->get_instr_ptr());
			printf(REG_FMT_STR(ANSI_BRIGHT_RED "r_stack_ptr"),
				(uint64_t) cpu->get_stack_ptr(), (uint64_t) cpu->get_stack_ptr());
			printf(REG_FMT_STR(ANSI_BRIGHT_RED "r_frame_ptr"),
				(uint64_t) cpu->get_frame_ptr(), (uint64_t) cpu->get_frame_ptr());
			printf(REG_FMT_STR(ANSI_BRIGHT_CYAN "r_ret      "),
				cpu->regs[R_RET], cpu->regs[R_RET]);
			printf(REG_FMT_STR(ANSI_BRIGHT_YELLOW "r_0        "),
				cpu->regs[R_0], cpu->regs[R_0]);
			printf(REG_FMT_STR(ANSI_BRIGHT_YELLOW "r_1        "),
				cpu->regs[R_1], cpu->regs[R_1]);
			printf(REG_FMT_STR(ANSI_BRIGHT_YELLOW "r_2        "),
				cpu->regs[R_2], cpu->regs[R_2]);
			printf(REG_FMT_STR(ANSI_BRIGHT_YELLOW "r_3        "),
				cpu->regs[R_3], cpu->regs[R_3]);
			printf(FLAG_FMT_STR(ANSI_BRIGHT_GREEN "greater_flag"),
				cpu->greater_flag);
			printf(FLAG_FMT_STR(ANSI_BRIGHT_GREEN "equal_flag  "),
				cpu->equal_flag);

#undef REG_FMT_STR
#undef FLAG_FMT_STR
		}

		// Show a list of all breakpoints set.

		else if (command[0] == "lsbp")
		{
			for (uint8_t *bp : breakpoints)
			{
				printf(ANSI_BRIGHT_MAGENTA ANSI_BOLD " - " ANSI_GREEN "0x" ANSI_BRIGHT_GREEN "%llx\n", (uint64_t) bp);
			}
		}

		// Set a new breakpoint.

		else if (command[0] == "bp")
		{
			if (command.num_of_args() < 2)
			{
				printf(ANSI_RED "Expected address for breakpoint\n");
			}
			else
			{
				std::string addr_str = command[1];
				uint8_t *address     = read_ptr(std::move(addr_str));
				breakpoints.insert(address);
			}
		}

		// Remove an existing breakpoint.

		else if (command[0] == "rmbp")
		{
			if (command.num_of_args() < 2)
			{
				printf(ANSI_RED "Expected address for breakpoint\n");
			}
			else
			{
				std::string addr_str = command[1];

				// If the argument is "all", remove all breakpoints.

				if (addr_str == "all" || addr_str == "*")
				{
					breakpoints.clear();
					goto shell;
				}

				// Else, remove a breakpoint by address.

				uint8_t *address = read_ptr(std::move(addr_str));
				breakpoints.erase(address);
			}
		}

		// Dump the current stack state.

		else if (command[0] == "ds")
		{
			if (cpu == NULL)
			{
				PROGRAM_NOT_STARTED_ERR();
				goto shell;
			}

			std::string top_flag = command.get_flag_value("top").c_str();
			size_t top;

			// Get the top flag value.

			if (top_flag == "")
			{
				top = DEFAULT_DUMP_STACK_TOP;
			}
			else if (top_flag == "*")
			{
				top = cpu->get_stack_ptr() - cpu->stack_top;
			}
			else
			{
				top = atoi(top_flag.c_str());
			}

			// Calculate the begin and end from the `top` argument given.

			uint8_t *begin = std::max(cpu->get_stack_ptr() - top, cpu->stack_top);
			uint8_t *end   = cpu->get_stack_ptr();

			memory::Reader reader(begin);

			// Read the stack and print each byte.

			while (reader.addr <= end /* && reader.is_safe() */)
			{
				uint8_t *addr = reader.addr;
				uint8_t byte  = reader.read<uint8_t>();

				if (addr == cpu->get_frame_ptr())
				{
					printf(ANSI_RED ANSI_BOLD "0x" ANSI_BRIGHT_RED "%04llx" ANSI_RESET
								  "    " ANSI_YELLOW "%03hhu" ANSI_RESET "    " ANSI_GREEN "0x" ANSI_BRIGHT_GREEN "%02hhx" ANSI_RESET "\n",
						(uint64_t) addr, byte, byte);
				}
				else if (addr == cpu->get_stack_ptr())
				{
					printf(ANSI_YELLOW ANSI_BOLD "0x" ANSI_BRIGHT_YELLOW "%04llx" ANSI_RESET
								     "    " ANSI_YELLOW "%03hhu" ANSI_RESET "    " ANSI_GREEN "0x" ANSI_BRIGHT_GREEN "%02hhx" ANSI_RESET "\n",
						(uint64_t) addr, byte, byte);
				}
				else
				{
					printf(ANSI_GREEN ANSI_BOLD "0x" ANSI_BRIGHT_GREEN "%04llx" ANSI_RESET
								    "    " ANSI_YELLOW "%03hhu" ANSI_RESET "    " ANSI_GREEN "0x" ANSI_BRIGHT_GREEN "%02hhx" ANSI_RESET "\n",
						(uint64_t) addr, byte, byte);
				}
			}
		}

		// Pretty print the current call stack.

		else if (command[0] == "cs")
		{
			if (cpu == NULL)
			{
				PROGRAM_NOT_STARTED_ERR();
				goto shell;
			}

			std::string top_flag = command.get_flag_value("top");
			size_t top;

			// Get the top flag value.

			if (top_flag == "")
			{
				top = DEFAULT_CALL_STACK_TOP;
			}
			else if (top_flag == "*")
			{
				top = call_stack.size();
			}
			else
			{
				top = atoi(top_flag.c_str());
			}

			size_t lower_bound;

			if (top > call_stack.size())
			{
				lower_bound = 0;
			}
			else
			{
				lower_bound = call_stack.size() - top;
			}

			for (size_t i = call_stack.size(); i != lower_bound; i--)
			{
				print_call_stack_entry(call_stack[i - 1], locals[i - 1], i - 1);

				if (i - 1 == lower_bound && lower_bound != 0)
				{
					printf(ANSI_BRIGHT_BLACK "..." ANSI_RESET "\n");
				}
			}
		}

		// Print the help message

		else if (command[0] == "help" || command[0] == "h")
		{
			if (command.num_of_args() > 1)
			{
				const std::string &cmd_name = command[1];

				if (cmd_name == "start")
				{
					print_help_start();
				}
				else if (cmd_name == "step" || cmd_name == "s")
				{
					print_help_step();
				}
				else if (cmd_name == "run" || cmd_name == "r")
				{
					print_help_run();
				}
				else if (cmd_name == "list" || cmd_name == "l")
				{
					print_help_list();
				}
				else if (cmd_name == "reg")
				{
					print_help_reg();
				}
				else if (cmd_name == "lsbp")
				{
					print_help_lsbp();
				}
				else if (cmd_name == "bp")
				{
					print_help_bp();
				}
				else if (cmd_name == "rmbp")
				{
					print_help_rmbp();
				}
				else if (cmd_name == "ds")
				{
					print_help_ds();
				}
				else if (cmd_name == "cs")
				{
					print_help_cs();
				}
				else if (cmd_name == "help" || cmd_name == "h")
				{
					print_help_help();
				}
				else
				{
					printf(ANSI_RED "Command \"%s\" not found" ANSI_RESET "\n",
						cmd_name.c_str());
				}

				goto shell;
			}

			print_help_start();
			print_help_step();
			print_help_run();
			print_help_list();
			print_help_reg();
			print_help_lsbp();
			print_help_bp();
			print_help_rmbp();
			print_help_ds();
			print_help_cs();
			print_help_help();
		}
		else
		{
			printf(ANSI_RED "Unknown command \"%s\". Try \"help\"" ANSI_RESET "\n",
				command[0].c_str());
		}

		goto shell;
	}

	void
	print_help_start()
	{
		printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "start" ANSI_RESET);
		printf(ANSI_CYAN " [ ");
		printf(ANSI_GREEN "--stack-size <number of bytes>");
		printf(ANSI_CYAN " ]" ANSI_RESET);
		printf(" - ");
		printf(ANSI_YELLOW "starts the program" ANSI_RESET "\n");
	}

	void
	print_help_step()
	{
		printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "step" ANSI_RESET);
		printf(" - ");
		printf(ANSI_YELLOW "executes the next instruction of the program" ANSI_RESET "\n");
	}

	void
	print_help_run()
	{
		printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "run" ANSI_RESET);
		printf(" - ");
		printf(ANSI_YELLOW "runs the program until the next breakpoint" ANSI_RESET "\n");
	}

	void
	print_help_list()
	{
		printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "list" ANSI_RESET);
		printf(ANSI_CYAN " [ ");
		printf(ANSI_GREEN "--top <number of instructions>");
		printf(ANSI_CYAN " ]" ANSI_RESET);
		printf(" - ");
		printf(ANSI_YELLOW "lists the next [top] instructions" ANSI_RESET "\n");
	}

	void
	print_help_reg()
	{
		printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "reg" ANSI_RESET);
		printf(" - ");
		printf(ANSI_YELLOW "lists the current state of the CPU registers" ANSI_RESET "\n");
	}

	void
	print_help_lsbp()
	{
		printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "lsbp" ANSI_RESET);
		printf(" - ");
		printf(ANSI_YELLOW "lists the addresses of the current set breakpoints" ANSI_RESET "\n");
	}

	void
	print_help_bp()
	{
		printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "bp" ANSI_RESET);
		printf(ANSI_GREEN " <address>" ANSI_RESET);
		printf(" - ");
		printf(ANSI_YELLOW "sets a breakpoint at address" ANSI_RESET "\n");
	}

	void
	print_help_rmbp()
	{
		printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "rmbp" ANSI_RESET);
		printf(ANSI_GREEN " <address | \"all\">" ANSI_RESET);
		printf(" - ");
		printf(ANSI_YELLOW "unsets a breakpoint at address, or all breakpoints" ANSI_RESET "\n");
	}

	void
	print_help_ds()
	{
		printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "ds" ANSI_RESET);
		printf(ANSI_CYAN " [ ");
		printf(ANSI_GREEN "--top <number of bytes>");
		printf(ANSI_CYAN " ]" ANSI_RESET);
		printf(" - ");
		printf(ANSI_YELLOW "dumps the first [top] bytes on the stack" ANSI_RESET "\n");
	}

	void
	print_help_cs()
	{
		printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "cs" ANSI_RESET);
		printf(" - ");
		printf(ANSI_YELLOW "prints the current call stack, including all variables" ANSI_RESET "\n");
	}

	void
	print_help_help()
	{
		printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "help" ANSI_RESET);
		printf(ANSI_CYAN " [ ");
		printf(ANSI_GREEN "<command name>");
		printf(ANSI_CYAN " ]" ANSI_RESET);
		printf(" - ");
		printf(ANSI_YELLOW "prints information about a command" ANSI_RESET "\n");
	}
};

int
main(int argc, char **argv)
{
	if (argc < 2)
	{
		fprintf(stderr, "Usage: ./debug program.teax\n");
		exit(1);
	}

	// Required to switch back from getch mode to normal terminal mode.

	keypress::save_termios();

	// Start the shell.

	char *file_path = argv[1];

	Shell shell(file_path);
	shell.run_shell();
}