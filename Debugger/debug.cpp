#include <bits/stdc++.h>
#include <sys/stat.h>

#define RESTORE_INSTRUCTION_POINTER_ON_THROW

#include "../ansi.hpp"
#include "../VM/cpu.hpp"
#include "../Assembler/assembler.hpp"
#include "util.hpp"
#include "command.hpp"
#include "instruction-lister.hpp"
#include "../Compiler/debugger-symbols.hpp"

#define DEFAULT_STACK_SIZE 2048
#define DEFAULT_LIST_TOP 15
#define DEFAULT_CALL_STACK_TOP 10
#define DEFAULT_DUMP_STACK_TOP 24

using namespace std;

#define PROGRAM_NOT_STARTED_ERR() \
	printf(ANSI_RED "Program has not yet started.\n" \
		"Start the program with the " ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD \
		"start" ANSI_RESET ANSI_RED " command.\n")

struct CallStackEntryArg {
	string arg_name;
	string value;
};

struct CallStackEntry {
	string fn_name;
	vector<CallStackEntryArg> args;
};

struct VarEntry : public DebuggerSymbol {
	uint64_t addr;

	VarEntry(const DebuggerSymbol& sym, uint64_t addr)
		: DebuggerSymbol(sym), addr(addr) {}
};

class Shell {
	private:
		const char *file_path;
		CPU *cpu = NULL;
		set<uint64_t> breakpoints;
		vector<CallStackEntry> call_stack;
		vector<vector<VarEntry>> locals;
		vector<VarEntry> globals;
		DebuggerSymbols debugger_symbols;
		bool debugger_symbols_found = false;

		void exec_instruction()
		{
			// Get the next instruction

			Instruction instruction = cpu->step();

			switch (instruction) {
				case CALL:
				{
					// Add this function call to call stack

					Instruction next_instruction = (Instruction) cpu->memory_mapper
						.get<uint16_t>(cpu->r_instruction_p);

					stringstream ss;

					// If the function has a debug label, push its name

					if (next_instruction == LABEL) {
						uint64_t offset = cpu->r_instruction_p + 2;
						char c;

						while (true) {
							c = cpu->memory_mapper.get<char>(offset++);
							if (c == '\0') break;
							ss << c;
						}
					}

					// Else, push its address

					else {
						ss << "0x" << hex << cpu->r_instruction_p;
					}

					// Create a call stack entry

					CallStackEntry entry;
					entry.fn_name = ss.str();

					// Check if there are debugger symbols

					if (debugger_symbols.functions.count(entry.fn_name)) {
						const DebuggerFunction& fn_symbols = debugger_symbols.functions[entry.fn_name];
						size_t params_offset = 0;

						// Calculate the size of the parameters

						for (const DebuggerSymbol& param : fn_symbols.params) {
							params_offset += param.byte_size();
						}

						// Store the argument values

						for (const DebuggerSymbol& param : fn_symbols.params) {
							CallStackEntryArg entry_arg;
							entry_arg.arg_name = param.name;
							size_t addr = cpu->r_frame_p - params_offset - 8 - CPU::stack_frame_size;

							switch (param.type) {
								case DebuggerSymbolTypes::POINTER:
								{
									uint64_t val = cpu->memory_mapper.get<uint64_t>(addr);
									entry_arg.value = ANSI_GREEN "0x" ANSI_BRIGHT_GREEN + to_hex_str(addr);
									break;
								}

								case DebuggerSymbolTypes::U8:
								{
									uint8_t val = cpu->memory_mapper.get<uint8_t>(addr);
									entry_arg.value = ANSI_YELLOW + to_string(val);
									break;
								}

								case DebuggerSymbolTypes::I8:
								{
									int8_t val = cpu->memory_mapper.get<int8_t>(addr);
									entry_arg.value = ANSI_YELLOW + to_string(val);
									break;
								}

								case DebuggerSymbolTypes::U16:
								{
									uint16_t val = cpu->memory_mapper.get<uint16_t>(addr);
									entry_arg.value = ANSI_YELLOW + to_string(val);
									break;
								}

								case DebuggerSymbolTypes::I16:
								{
									int16_t val = cpu->memory_mapper.get<int16_t>(addr);
									entry_arg.value = ANSI_YELLOW + to_string(val);
									break;
								}

								case DebuggerSymbolTypes::U32:
								{
									uint32_t val = cpu->memory_mapper.get<uint32_t>(addr);
									entry_arg.value = ANSI_YELLOW + to_string(val);
									break;
								}

								case DebuggerSymbolTypes::I32:
								{
									int32_t val = cpu->memory_mapper.get<int32_t>(addr);
									entry_arg.value = ANSI_YELLOW + to_string(val);
									break;
								}

								case DebuggerSymbolTypes::I64:
								{
									int64_t val = cpu->memory_mapper.get<int64_t>(addr);
									entry_arg.value = ANSI_YELLOW + to_string(val);
									break;
								}

								case DebuggerSymbolTypes::U64:
								{
									uint64_t val = cpu->memory_mapper.get<uint64_t>(addr);
									entry_arg.value = ANSI_YELLOW + to_string(val);
									break;
								}
							}

							params_offset -= param.byte_size();
							entry.args.push_back(entry_arg);
						}

						// Store addresses of locals

						vector<VarEntry> new_fn_locals;
						size_t addr = cpu->r_stack_p;

						for (const DebuggerSymbol& local : fn_symbols.locals) {
							new_fn_locals.push_back(VarEntry(local, addr));
							addr += local.byte_size();
						}

						locals.push_back(new_fn_locals);
					}

					call_stack.push_back(entry);
					break;
				}

				case RETURN:
				{
					// Pop from the call stack and the locals

					call_stack.pop_back();
					locals.pop_back();
				}

				default:;
			}
		}

		void step()
		{
			// Execute the next instruction, if the program is still running

			if (cpu->r_instruction_p < cpu->stack_top()) {
				try {
					exec_instruction();
				} catch (const string& err_message) {
					printf(ANSI_RED "The VM encountered an error:\n" ANSI_BRIGHT_RED "%s",
						err_message.c_str());
				}
			} else {
				printf(ANSI_RED "Program ended, cannot step\n");
			}
		}

		void run()
		{
			// Execute the next instruction

			if (cpu->r_instruction_p < cpu->stack_top()) {
				try {
					exec_instruction();
				} catch (const string& err_message) {
					printf(ANSI_RED "The VM encountered an error:\n" ANSI_BRIGHT_RED "%s",
						err_message.c_str());
					return;
				}
			}

			// And keep executing until a breakpoint is hit, or the program finishes

			while (cpu->r_instruction_p < cpu->stack_top()) {
				if (breakpoints.count(cpu->r_instruction_p)) return;

				try {
					exec_instruction();
				} catch (const string& err_message) {
					printf(ANSI_RED "The VM encountered an error:\n" ANSI_BRIGHT_RED "%s",
						err_message.c_str());
					return;
				}
			}

			// Print the exit code

			printf(ANSI_CYAN "VM exited with exit code " ANSI_YELLOW "%lu\n",
				cpu->r_accumulator_0);
		}

	public:
		Shell(const char *file_path) : file_path(file_path) {}

		void run_shell()
		{
			shell:

			// Show the prompt and read the command from the user

			Command command = Command::read(file_path, cpu == NULL ? -1 : cpu->r_instruction_p);

			// If the command is empty, try again

			if (command.num_of_args() == 0) goto shell;

			// Load the program

			if (command[0] == "start") {
				string stack_size_flag = command.get_flag_value("stack-size").c_str();
				size_t stack_size;

				if (stack_size_flag == "") stack_size = DEFAULT_STACK_SIZE;
				else stack_size = atoi(stack_size_flag.c_str());

				Executable executable = Executable::from_file(file_path);
				cpu = new CPU(executable, stack_size);

				printf(ANSI_BRIGHT_MAGENTA "Loaded executable" ANSI_RESET "\n");

				string debug_symbols_filename;
				debug_symbols_filename += file_path;
				debug_symbols_filename += ".debug";

				struct stat file_stats;

				// Read debugger symbols if found

				if (stat(debug_symbols_filename.c_str(), &file_stats) == 0) {
					printf(ANSI_BRIGHT_MAGENTA "Found debugger symbols" ANSI_RESET "\n");
					debugger_symbols = DebuggerSymbols::parse(debug_symbols_filename);
					debugger_symbols_found = true;
				} else {
					debugger_symbols_found = false;
				}

				// Store addresses of globals

				size_t addr = cpu->stack_top();

				for (const DebuggerSymbol& sym : debugger_symbols.globals) {
					globals.push_back(VarEntry(sym, addr));
					addr += sym.byte_size();
				}
			}

			// Execute the next instruction of the program

			else if (command[0] == "step" || command[0] == "s") {
				if (cpu == NULL) {
					PROGRAM_NOT_STARTED_ERR();
					goto shell;
				}

				step();
			}

			// Execute the program until the next breakpoint is hit, or the program ends

			else if (command[0] == "run" || command[0] == "r") {
				if (cpu == NULL) {
					PROGRAM_NOT_STARTED_ERR();
					goto shell;
				}

				run();
			}

			// Pretty print the CPU instructions

			else if (command[0] == "list" || command[0] == "l") {
				if (cpu == NULL) {
					PROGRAM_NOT_STARTED_ERR();
					goto shell;
				}

				string top_flag = command.get_flag_value("top").c_str();
				size_t top;

				MemoryMapperReader reader(cpu->memory_mapper, cpu->r_instruction_p);
				InstructionLister lister(reader);

				// Get the top flag value

				if (top_flag == "") top = DEFAULT_LIST_TOP;
				else if (top_flag == "*") {
					lister.disassemble_all(cpu->stack_top(), breakpoints);
					goto shell;
				}
				else top = atoi(top_flag.c_str());

				// Dissassemble the program

				lister.disassemble(top, breakpoints);
			}

			// Pretty print the current CPU register states

			else if (command[0] == "registers" || command[0] == "reg") {
				if (cpu == NULL) {
					PROGRAM_NOT_STARTED_ERR();
					goto shell;
				}

				#define REG_FMT_STR(reg) ANSI_BOLD ANSI_ITALIC reg ANSI_RESET " = " \
					ANSI_GREEN "0x" ANSI_BRIGHT_GREEN "%016lx" ANSI_RESET " = " \
					ANSI_YELLOW "%020lu" ANSI_RESET "\n"

				#define FLAG_FMT_STR(flag) ANSI_BOLD ANSI_ITALIC flag ANSI_RESET " = " \
					ANSI_YELLOW "%hhu" ANSI_RESET "\n"

				printf(REG_FMT_STR(ANSI_BRIGHT_RED    "r_instruction_p"),
					cpu->r_instruction_p, cpu->r_instruction_p);
				printf(REG_FMT_STR(ANSI_BRIGHT_RED    "r_stack_p      "),
					cpu->r_stack_p, cpu->r_stack_p);
				printf(REG_FMT_STR(ANSI_BRIGHT_RED    "r_frame_p      "),
					cpu->r_frame_p, cpu->r_frame_p);
				printf(REG_FMT_STR(ANSI_BRIGHT_CYAN   "r_accumulator_0"),
					cpu->r_accumulator_0, cpu->r_accumulator_0);
				printf(REG_FMT_STR(ANSI_BRIGHT_CYAN   "r_accumulator_1"),
					cpu->r_accumulator_1,cpu-> r_accumulator_1);
				printf(REG_FMT_STR(ANSI_BRIGHT_YELLOW "r_0            "),
					cpu->r_0, cpu->r_0);
				printf(REG_FMT_STR(ANSI_BRIGHT_YELLOW "r_1            "),
					cpu->r_1, cpu->r_1);
				printf(REG_FMT_STR(ANSI_BRIGHT_YELLOW "r_2            "),
					cpu->r_2, cpu->r_2);
				printf(REG_FMT_STR(ANSI_BRIGHT_YELLOW "r_3            "),
					cpu->r_3, cpu->r_3);
				printf(FLAG_FMT_STR(ANSI_BRIGHT_GREEN "greater_flag"),
					cpu->greater_flag);
				printf(FLAG_FMT_STR(ANSI_BRIGHT_GREEN "equal_flag  "),
					cpu->equal_flag);

				#undef REG_FMT_STR
				#undef FLAG_FMT_STR
			}

			// Show a list of all breakpoints set

			else if (command[0] == "lsbp") {
				for (uint64_t bp : breakpoints) {
					printf(ANSI_BRIGHT_MAGENTA ANSI_BOLD " - "
						ANSI_GREEN "0x" ANSI_BRIGHT_GREEN "%lx\n", bp);
				}
			}

			// Set a new breakpoint

			else if (command[0] == "bp") {
				if (command.num_of_args() < 2) {
					printf(ANSI_RED "Expected address for breakpoint\n");
				} else {
					string addr_str = command[1];
					uint64_t address;
					bool hexadecimal = false;

					// If the address starts with "0x" or "x", use hex format

					if (addr_str.size() >= 1 && addr_str[0] == 'x') {
						addr_str = addr_str.substr(1);
						hexadecimal = true;
					} else if (addr_str.size() >= 2 && addr_str[0] == '0' && addr_str[1] == 'x') {
						addr_str = addr_str.substr(2);
						hexadecimal = true;
					}

					// Read the address

					stringstream ss;

					if (hexadecimal) {
						ss << hex;
					}

					ss << addr_str;
					ss >> address;

					// Insert the address into the breakpoints set

					breakpoints.insert(address);
				}
			}

			// Remove an existing breakpoint

			else if (command[0] == "rmbp") {
				string addr_str = command[1];
				uint64_t address;
				bool hexadecimal = false;

				// If the argument is "all", remove all breakpoints

				if (addr_str == "all" || addr_str == "*") {
					breakpoints.clear();
					goto shell;
				}

				// If the address starts with "0x" or "x", use hex format

				if (addr_str.size() >= 1 && addr_str[0] == 'x') {
					addr_str = addr_str.substr(1);
					hexadecimal = true;
				} else if (addr_str.size() >= 2 && addr_str[0] == '0' && addr_str[1] == 'x') {
					addr_str = addr_str.substr(2);
					hexadecimal = true;
				}

				// Read the address

				stringstream ss;

				if (hexadecimal) {
					ss << hex;
				}

				ss << addr_str;
				ss >> address;

				// Remove the address from the breakpoints set

				breakpoints.erase(address);
			}

			// Dump the current stack state

			else if (command[0] == "ds") {
				if (cpu == NULL) {
					PROGRAM_NOT_STARTED_ERR();
					goto shell;
				}

				string top_flag = command.get_flag_value("top").c_str();
				size_t top;

				// Get the top flag value

				if (top_flag == "") top = DEFAULT_DUMP_STACK_TOP;
				else if (top_flag == "*") top = cpu->r_stack_p - cpu->stack_top();
				else top = atoi(top_flag.c_str());

				// Calculate the begin and end from the `top` argument given

				uint64_t begin = max(cpu->r_stack_p - top, cpu->stack_top());
				uint64_t end = cpu->r_stack_p;

				MemoryMapperReader reader(cpu->memory_mapper, begin);

				// Read the stack and print each byte

				while (reader.offset <= end && reader.is_safe()) {
					uint64_t addr = reader.offset;
					uint8_t byte = reader.read<uint8_t>();

					if (addr == cpu->r_frame_p) {
						printf(ANSI_RED ANSI_BOLD "0x" ANSI_BRIGHT_RED "%04lx" ANSI_RESET
							"    " ANSI_YELLOW "%03hhu" ANSI_RESET "    "
							ANSI_GREEN "0x" ANSI_BRIGHT_GREEN "%02hhx" ANSI_RESET "\n",
							addr, byte, byte);
					} else if (addr == cpu->r_stack_p) {
						printf(ANSI_YELLOW ANSI_BOLD "0x" ANSI_BRIGHT_YELLOW "%04lx" ANSI_RESET
							"    " ANSI_YELLOW "%03hhu" ANSI_RESET "    "
							ANSI_GREEN "0x" ANSI_BRIGHT_GREEN "%02hhx" ANSI_RESET "\n",
							addr, byte, byte);
					} else {
						printf(ANSI_GREEN ANSI_BOLD "0x" ANSI_BRIGHT_GREEN "%04lx" ANSI_RESET
							"    " ANSI_YELLOW "%03hhu" ANSI_RESET "    "
							ANSI_GREEN "0x" ANSI_BRIGHT_GREEN "%02hhx" ANSI_RESET "\n",
							addr, byte, byte);
					}
				}
			}

			// Pretty print the current call stack

			else if (command[0] == "cs") {
				if (cpu == NULL) {
					PROGRAM_NOT_STARTED_ERR();
					goto shell;
				}

				string top_flag = command.get_flag_value("top");
				size_t top;

				// Get the top flag value

				if (top_flag == "") top = DEFAULT_CALL_STACK_TOP;
				else if (top_flag == "*") top = call_stack.size();
				else top = atoi(top_flag.c_str());

				size_t lower_bound;

				if (top > call_stack.size()) lower_bound = 0;
				else lower_bound = call_stack.size() - top;

				for (size_t i = call_stack.size(); i != lower_bound; i--) {
					CallStackEntry entry = call_stack[i - 1];
					const vector<VarEntry>& cur_locals = locals[i - 1];

					// Print function name

					printf(ANSI_BRIGHT_MAGENTA ANSI_BOLD "  # %lu "
						ANSI_BRIGHT_BLUE "%s" ANSI_BRIGHT_BLACK "(" ANSI_RESET,
						i - 1, entry.fn_name.c_str());

					// Print args

					for (size_t i = 0; i < entry.args.size(); i++) {
						// Print arg name and value

						printf(ANSI_BRIGHT_RED ANSI_ITALIC "%s"
							ANSI_RESET ANSI_BRIGHT_BLACK " = %s" ANSI_RESET,
							entry.args[i].arg_name.c_str(), entry.args[i].value.c_str());

						// Print comma seperator

						if (i != entry.args.size() - 1) {
							printf(ANSI_BRIGHT_BLACK ", ");
						}
					}

					// Print closing parenthesis

					printf(ANSI_BRIGHT_BLACK ANSI_BOLD ")" ANSI_RESET "\n");

					// Print locals

					for (size_t i = 0; i < cur_locals.size(); i++) {
						const VarEntry& local = cur_locals[i];
						const char *conn_chars;

						if (i == cur_locals.size() - 1) conn_chars = "╰─";
						else conn_chars = "├─";

						#define PRINT_VAR(printf_flag, val) printf(ANSI_BRIGHT_MAGENTA ANSI_BOLD \
							"    %s " ANSI_BLUE "%s" ANSI_RESET ANSI_BRIGHT_BLACK " = " \
							ANSI_YELLOW printf_flag ANSI_RESET "\n", \
							conn_chars, local.name.c_str(), val)

						switch (local.type) {
							case DebuggerSymbolTypes::U8:
							{
								uint8_t val = cpu->memory_mapper.get<uint8_t>(local.addr);
								PRINT_VAR("%hhu", val);
								break;
							}

							case DebuggerSymbolTypes::I8:
							{
								int8_t val = cpu->memory_mapper.get<int8_t>(local.addr);
								PRINT_VAR("%hhd", val);
								break;
							}

							case DebuggerSymbolTypes::U16:
							{
								uint16_t val = cpu->memory_mapper.get<uint16_t>(local.addr);
								PRINT_VAR("%hu", val);
								break;
							}

							case DebuggerSymbolTypes::I16:
							{
								int16_t val = cpu->memory_mapper.get<int16_t>(local.addr);
								PRINT_VAR("%hd", val);
								break;
							}

							case DebuggerSymbolTypes::U32:
							{
								uint32_t val = cpu->memory_mapper.get<uint32_t>(local.addr);
								PRINT_VAR("%u", val);
								break;
							}

							case DebuggerSymbolTypes::I32:
							{
								int32_t val = cpu->memory_mapper.get<int32_t>(local.addr);
								PRINT_VAR("%d", val);
								break;
							}

							case DebuggerSymbolTypes::U64:
							{
								uint64_t val = cpu->memory_mapper.get<uint64_t>(local.addr);
								PRINT_VAR("%lu", val);
								break;
							}

							case DebuggerSymbolTypes::I64:
							{
								int64_t val = cpu->memory_mapper.get<int64_t>(local.addr);
								PRINT_VAR("%ld", val);
								break;
							}

							case DebuggerSymbolTypes::POINTER:
							{
								int64_t val = cpu->memory_mapper.get<uint64_t>(local.addr);
								printf(ANSI_BRIGHT_MAGENTA ANSI_BOLD
									"    - " ANSI_BLUE "%s" ANSI_RESET ANSI_BRIGHT_BLACK " = "
									ANSI_GREEN "0x" ANSI_BRIGHT_GREEN "%s" ANSI_RESET "\n",
									local.name.c_str(), to_hex_str(val).c_str());
								break;
							}

							case DebuggerSymbolTypes::USER_DEFINED_CLASS:
							{
								// Todo: create

								printf(ANSI_BRIGHT_MAGENTA ANSI_BOLD
									"    - " ANSI_BLUE "%s" ANSI_RESET ANSI_BRIGHT_BLACK " = "
									ANSI_YELLOW "... (not implemented)" ANSI_RESET "\n",
									local.name.c_str());
								break;
							}

							default:
							{
								printf(ANSI_BRIGHT_MAGENTA ANSI_BOLD
									"    - " ANSI_BLUE "???" ANSI_RESET ANSI_BRIGHT_BLACK " = "
									ANSI_YELLOW "???" ANSI_RESET "\n");
								break;
							}
						}

						#undef PRINT_VAR
					}

					if (i - 1 == lower_bound && lower_bound != 0)
						printf(ANSI_BRIGHT_BLACK "..." ANSI_RESET "\n");
				}
			}

			// Print the help message

			else if (command[0] == "help" || command[0] == "h") {
				if (command.num_of_args() > 1) {
					const string& cmd_name = command[1];

					if (cmd_name == "start") print_help_start();
					else if (cmd_name == "step" || cmd_name == "s") print_help_step();
					else if (cmd_name == "run" || cmd_name == "r") print_help_run();
					else if (cmd_name == "list" || cmd_name == "l") print_help_list();
					else if (cmd_name == "reg") print_help_reg();
					else if (cmd_name == "lsbp") print_help_lsbp();
					else if (cmd_name == "bp") print_help_bp();
					else if (cmd_name == "rmbp") print_help_rmbp();
					else if (cmd_name == "ds") print_help_ds();
					else if (cmd_name == "cs") print_help_cs();
					else if (cmd_name == "help" || cmd_name == "h") print_help_help();
					else printf(ANSI_RED "Command \"%s\" not found" ANSI_RESET "\n",
						cmd_name.c_str());

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
			} else {
				printf(ANSI_RED "Unknown command \"%s\". Try \"help\"" ANSI_RESET "\n",
					command[0].c_str());
			}

			goto shell;
		}

		void print_help_start()
		{
			printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "start" ANSI_RESET);
			printf(ANSI_CYAN " [ ");
			printf(ANSI_GREEN "--stack-size <number of bytes>");
			printf(ANSI_CYAN " ]" ANSI_RESET);
			printf(" - ");
			printf(ANSI_YELLOW "starts the program" ANSI_RESET "\n");
		}

		void print_help_step()
		{
			printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "step" ANSI_RESET);
			printf(" - ");
			printf(ANSI_YELLOW "executes the next instruction of the program" ANSI_RESET "\n");
		}

		void print_help_run()
		{
			printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "run" ANSI_RESET);
			printf(" - ");
			printf(ANSI_YELLOW "runs the program until the next breakpoint" ANSI_RESET "\n");
		}

		void print_help_list()
		{
			printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "list" ANSI_RESET);
			printf(ANSI_CYAN " [ ");
			printf(ANSI_GREEN "--top <number of instructions>");
			printf(ANSI_CYAN " ]" ANSI_RESET);
			printf(" - ");
			printf(ANSI_YELLOW "lists the next [top] instructions" ANSI_RESET "\n");
		}

		void print_help_reg()
		{
			printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "reg" ANSI_RESET);
			printf(" - ");
			printf(ANSI_YELLOW "lists the current state of the CPU registers" ANSI_RESET "\n");
		}

		void print_help_lsbp()
		{
			printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "lsbp" ANSI_RESET);
			printf(" - ");
			printf(ANSI_YELLOW "lists the addresses of the current set breakpoints" ANSI_RESET "\n");
		}

		void print_help_bp()
		{
			printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "bp" ANSI_RESET);
			printf(ANSI_GREEN " <address>" ANSI_RESET);
			printf(" - ");
			printf(ANSI_YELLOW "sets a breakpoint at address" ANSI_RESET "\n");
		}

		void print_help_rmbp()
		{
			printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "rmbp" ANSI_RESET);
			printf(ANSI_GREEN " <address | \"all\">" ANSI_RESET);
			printf(" - ");
			printf(ANSI_YELLOW "unsets a breakpoint at address, or all breakpoints" ANSI_RESET "\n");
		}

		void print_help_ds()
		{
			printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "ds" ANSI_RESET);
			printf(ANSI_CYAN " [ ");
			printf(ANSI_GREEN "--top <number of bytes>");
			printf(ANSI_CYAN " ]" ANSI_RESET);
			printf(" - ");
			printf(ANSI_YELLOW "dumps the first [top] bytes on the stack" ANSI_RESET "\n");
		}

		void print_help_cs()
		{
			printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "cs" ANSI_RESET);
			printf(" - ");
			printf(ANSI_YELLOW "prints the current call stack, including all variables" ANSI_RESET "\n");
		}

		void print_help_help()
		{
			printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "help" ANSI_RESET);
			printf(ANSI_CYAN " [ ");
			printf(ANSI_GREEN "<command name>");
			printf(ANSI_CYAN " ]" ANSI_RESET);
			printf(" - ");
			printf(ANSI_YELLOW "prints information about a command" ANSI_RESET "\n");
		}
};

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "Usage: ./debug program.teax\n");
		exit(1);
	}

	// Required to switch back from getch mode to normal terminal mode

	keypress::save_termios();

	// Start the shell

	char *file_path = argv[1];

	Shell shell(file_path);
	shell.run_shell();
}