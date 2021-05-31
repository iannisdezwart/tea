#include <bits/stdc++.h>

#include "../ansi.hpp"
#include "keypress.hpp"
#include "../VM/cpu.hpp"
#include "../VM/memory-mapper.hpp"
#include "../Assembler/assembler.hpp"

#define DEFAULT_STACK_SIZE 2048
#define DEFAULT_LIST_TOP 15
#define DEFAULT_DUMP_STACK_TOP 24

using namespace std;

bool starts_with(const string& str, const string& search)
{
	if (search.size() > str.size()) return false;

	for (size_t i = 0; i < search.size(); i++) {
		if (str[i] != search[i]) return false;
	}

	return true;
}

#define is_whitespace(c) (c == ' ' || c == '\t' || c == '\n')

void print_shell_prompt(const char *file_path, uint64_t addr)
{
	printf(ANSI_CYAN ANSI_BOLD "%s" ANSI_RESET, file_path);

	if (addr != -1) {
		printf(ANSI_YELLOW " [ ");
		printf(ANSI_GREEN ANSI_BOLD "0x" ANSI_BRIGHT_GREEN "%04lx" ANSI_RESET, addr);
		printf(ANSI_YELLOW " ]");
	}

	printf(ANSI_BRIGHT_MAGENTA ANSI_BOLD " ➜ " ANSI_RESET);
}

class CommandHistory {
	private:
		vector<string> history;
		size_t i = 0;

	public:
		void push(const string& line)
		{
			history.push_back(line);
		}

		size_t size()
		{
			return history.size();
		}

		bool at_top()
		{
			return i == 0;
		}

		bool at_bottom()
		{
			return i == history.size() - 1;
		}

		void scroll_up()
		{
			if (i != 0) i--;
		}

		void scroll_down()
		{
			if (i != history.size() - 1) i++;
		}

		void scroll_to_bottom()
		{
			i = history.size() - 1;
		}

		const string& get()
		{
			return history[i];
		}

		void update_bottom(const string& new_line)
		{
			history[history.size() - 1] = new_line;
		}
};

CommandHistory command_history;

class Command {
	private:
		const string& command_line;

		void split_command_line()
		{
			size_t start;
			bool parsing = false;
			char quotes = '\0';

			for (size_t i = 0; i < command_line.size(); i++) {
				if (!parsing) {
					if (command_line[i] == ' ') continue;
					if (command_line[i] == '"') quotes = '"';

					parsing = true;
					start = i;
				} else {
					if (
						command_line[i] == ' ' && !quotes
						|| quotes && command_line[i] == quotes
					) {
						parsing = false;
						args.push_back(command_line.substr(start, i - start));
					}
				}
			}

			if (parsing) {
				args.push_back(command_line.substr(start));
			}
		}

	public:
		vector<string> args;

		Command(const string& command_line) : command_line(command_line)
		{
			split_command_line();
		}

		bool has_flag(const string& flag)
		{
			if (flag.size() == 1) {
				for (size_t i = 0; i < args.size(); i++) {
					if (args[i].size() >= 2 && args[i][0] == '-' && args[i][1] != '-') {
						for (size_t j = 0; j < args[i].size(); j++) {
							if (args[i][j] == flag[0]) return true;
						}
					}
				}
			} else {
				for (size_t i = 0; i < args.size(); i++) {
					if (args[i].size() >= 3 && args[i][0] == '-' && args[i][1] == '-') {
						string arg = args[i].substr(2);
						if (arg == flag) return true;
					}
				}
			}

			return false;
		}

		string get_flag_value(const string& flag)
		{
			for (size_t i = 0; i < args.size(); i++) {
				if (args[i].size() >= 3 && args[i][0] == '-' && args[i][1] == '-') {
					string arg = args[i].substr(2);
					if (arg == flag) {
						if (i == args.size() - 1) return "";
						return args[i + 1];
					}
				}
			}

			return "";
		}

		const string& operator[](size_t i)
		{
			return args[i];
		}

		size_t num_of_args()
		{
			return args.size();
		}

		static Command read(const char *file_path, uint64_t addr)
		{
			string line;
			size_t index = 0;

			command_history.push(line);
			command_history.scroll_to_bottom();
			keypress::start_getch_mode();

			while (true) {
				printf(ANSI_ERASE_LINE(2) ANSI_CURSOR_TO_COL(1));
				print_shell_prompt(file_path, addr);
				printf("%s", line.c_str());

				if (line.size() != 0 && index != line.size()) {
					ansi_cursor_back(line.size() - index);
				}

				uint8_t key = keypress::get_key();
				bool at_very_left = index == 0;
				bool at_very_right = index == line.size();

				switch (key) {
					case keypress::BACKSPACE:
						if (!at_very_left) {
							line.erase(index - 1, 1);
							index--;
							command_history.update_bottom(line);
						}

						break;

					case keypress::CTRL_BACKSPACE:
					case keypress::ALT_BACKSPACE:
						if (!at_very_left) {
							size_t space_index = 0;
							bool found_word = false;

							for (size_t i = index; i != 0; i--) {
								char c = line[i - 1];
								bool whitespace = is_whitespace(c);

								if (whitespace && found_word) {
									space_index = i;
									break;
								}

								if (!whitespace) found_word = true;
							}

							size_t length = index - space_index;
							line.erase(space_index, length);
							index -= length;
							command_history.update_bottom(line);
						}

						break;

					case keypress::DELETE:
						if (!at_very_right) {
							line.erase(index, 1);
							command_history.update_bottom(line);
						}

						break;

					case keypress::ARROW_LEFT:
						if (!at_very_left) index--;
						break;

					case keypress::ARROW_RIGHT:
						if (!at_very_right) index++;
						break;

					case keypress::ARROW_UP:
						if (!command_history.at_top()) {
							command_history.scroll_up();
							line = command_history.get();
							index = line.size();
						}

						break;

					case keypress::ARROW_DOWN:
						if (!command_history.at_bottom()) {
							command_history.scroll_down();
							line = command_history.get();
							index = line.size();
						}

						break;

					case keypress::ENTER:
						printf("\n");
						goto run_command;

					default:
						line.insert(index, 1, key);
						command_history.update_bottom(line);
						index++;
				}
			}

			run_command:

			keypress::stop_getch_mode();
			return Command(line);
		}
};

class InstructionLister {
	public:
		MemoryMapperReader& reader;
		bool first_arg;

		InstructionLister(MemoryMapperReader& reader)
			: reader(reader), first_arg(true) {}

		void print_instruction(const char *instruction, set<uint64_t> breakpoints)
		{
			// Print address in green

			uint64_t address = reader.offset - 2;

			if (breakpoints.count(address)) {
				printf(ANSI_RED ANSI_BOLD "0x" ANSI_BRIGHT_RED "%04lx" ANSI_RESET "    ",
					address);
			} else {
				printf(ANSI_GREEN ANSI_BOLD "0x" ANSI_BRIGHT_GREEN "%04lx" ANSI_RESET "    ",
					address);
			}

			// Print instruction in orange

			printf(ANSI_YELLOW ANSI_ITALIC "%-33s" ANSI_RESET, instruction);
		}

		void print_arg()
		{
			if (first_arg) {
				first_arg = false;
				printf(" ");
			} else {
				printf(", ");
			}
		}

		void print_arg_reg(uint8_t reg_id)
		{
			print_arg();
			printf(ANSI_CYAN ANSI_ITALIC "%s" ANSI_RESET, CPU::reg_to_str(reg_id));
		}

		void print_arg_literal_number(uint64_t num)
		{
			print_arg();
			printf(ANSI_YELLOW "%ld" ANSI_RESET, num);
		}

		void print_arg_address(uint64_t address)
		{
			print_arg();
			printf(ANSI_GREEN ANSI_BOLD "0x" ANSI_BRIGHT_GREEN "%lx" ANSI_RESET, address);
		}

		void end_args()
		{
			printf("\n");
			first_arg = true;
		}

		void disassemble(size_t num_of_instructions, set<uint64_t> breakpoints)
		{
			for (size_t i = 0; i < num_of_instructions; i++) {
				if (!reader.is_safe()) break;

				Instruction instruction = (Instruction) reader.read<uint16_t>();
				const char * instruction_str = instruction_to_str(instruction);
				vector<ArgumentType> args = instruction_arg_types(instruction);

				print_instruction(instruction_str, breakpoints);

				for (ArgumentType arg : args) {
					switch (arg) {
						case REG:
							print_arg_reg(reader.read<uint8_t>());
							break;

						case ADDR:
							print_arg_address(reader.read<uint64_t>());
							break;

						case LIT_8:
							print_arg_literal_number(reader.read<uint8_t>());
							break;

						case LIT_16:
							print_arg_literal_number(reader.read<uint16_t>());
							break;

						case LIT_32:
							print_arg_literal_number(reader.read<uint32_t>());
							break;

						case LIT_64:
							print_arg_literal_number(reader.read<uint64_t>());
							break;

						default:
							fprintf(stderr, "I think I messed up the code again ;-;");
							abort();
					}
				}

				end_args();
			}
		}
};

CPU *start(const string& file_path, size_t stack_size)
{
	Executable executable = Executable::from_file(file_path.c_str());
	return new CPU(executable, stack_size);
}

void step(CPU *cpu)
{
	if (cpu->r_instruction_p < cpu->stack_top()) {
		cpu->step();
	} else {
		printf("Program ended, cannot step\n");
	}
}

void run(CPU *cpu, set<uint64_t> breakpoints)
{
	if (cpu->r_instruction_p < cpu->stack_top()) {
		cpu->step();
	}

	while (cpu->r_instruction_p < cpu->stack_top()) {
		if (breakpoints.count(cpu->r_instruction_p)) return;
		cpu->step();
	}

	printf("VM exited with exit code %lu\n", cpu->r_accumulator_0);
}

void shell(const char *file_path)
{
	CPU *cpu = NULL;
	set<uint64_t> breakpoints;

	shell:

	Command command = Command::read(file_path, cpu == NULL ? -1 : cpu->r_instruction_p);

	if (command.num_of_args() == 0) goto shell;

	if (command[0] == "start") {
		string stack_size_flag = command.get_flag_value("stack-size").c_str();
		size_t stack_size;

		if (stack_size_flag == "") stack_size = DEFAULT_STACK_SIZE;
		else stack_size = atoi(stack_size_flag.c_str());

		cpu = start(file_path, stack_size);
	}

	else if (command[0] == "step" || command[0] == "s") {
		if (cpu == NULL) {
			printf(ANSI_RED "Program has not yet started.\n"
				"Start the program with the " ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD
				"start" ANSI_RESET ANSI_RED " command.\n");
		} else {
			step(cpu);
		}
	}

	else if (command[0] == "run" || command[0] == "r") {
		if (cpu == NULL) {
			printf(ANSI_RED "Program has not yet started.\n"
				"Start the program with the " ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD
				"start" ANSI_RESET ANSI_RED " command.\n");
		} else {
			run(cpu, breakpoints);
		}
	}

	else if (command[0] == "list" || command[0] == "l") {
		if (cpu == NULL) {
			printf(ANSI_RED "Program has not yet started.\n"
				"Start the program with the " ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD
				"start" ANSI_RESET ANSI_RED " command.\n");
		} else {
			string top_flag = command.get_flag_value("top").c_str();
			size_t top;

			if (top_flag == "") top = DEFAULT_LIST_TOP;
			else top = atoi(top_flag.c_str());

			MemoryMapperReader reader(cpu->memory_mapper, cpu->r_instruction_p);
			InstructionLister lister(reader);

			lister.disassemble(top, breakpoints);
		}
	}

	else if (command[0] == "registers" || command[0] == "reg") {
		if (cpu == NULL) {
			printf(ANSI_RED "Program has not yet started.\n"
				"Start the program with the " ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD
				"start" ANSI_RESET ANSI_RED " command.\n");
		} else {
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
	}

	else if (command[0] == "lsbp") {
		for (uint64_t bp : breakpoints) {
			printf(ANSI_BRIGHT_MAGENTA ANSI_BOLD " - "
				ANSI_GREEN "0x" ANSI_BRIGHT_GREEN "%lx\n", bp);
		}
	}

	else if (command[0] == "bp") {
		if (command.num_of_args() < 2) {
			printf(ANSI_RED "Expected address for breakpoint\n");
		} else {
			string addr_str = command[1];
			uint64_t address;
			bool hexadecimal = false;

			if (addr_str.size() >= 1 && addr_str[0] == 'x') {
				addr_str = addr_str.substr(1);
				hexadecimal = true;
			} else if (addr_str.size() >= 2 && addr_str[0] == '0' && addr_str[1] == 'x') {
				addr_str = addr_str.substr(2);
				hexadecimal = true;
			}

			stringstream ss;

			if (hexadecimal) {
				ss << hex;
			}

			ss << addr_str;
			ss >> address;

			breakpoints.insert(address);
		}
	}

	else if (command[0] == "rmbp") {
		string addr_str = command[1];
		uint64_t address;
		bool hexadecimal = false;

		if (addr_str.size() >= 1 && addr_str[0] == 'x') {
			addr_str = addr_str.substr(1);
			hexadecimal = true;
		} else if (addr_str.size() >= 2 && addr_str[0] == '0' && addr_str[1] == 'x') {
			addr_str = addr_str.substr(2);
			hexadecimal = true;
		}

		stringstream ss;

		if (hexadecimal) {
			ss << hex;
		}

		ss << addr_str;
		ss >> address;

		breakpoints.erase(address);
	}

	else if (command[0] == "ds") {
		string top_flag = command.get_flag_value("top").c_str();
		size_t top;

		if (top_flag == "") top = DEFAULT_LIST_TOP;
		else top = atoi(top_flag.c_str());

		uint64_t begin = max(cpu->r_stack_p - top, cpu->stack_top());
		uint64_t end = cpu->r_stack_p;

		MemoryMapperReader reader(cpu->memory_mapper, begin);

		while (reader.offset < end && reader.is_safe()) {
			uint64_t addr = reader.offset;
			uint8_t byte = reader.read<uint8_t>();

			printf(ANSI_GREEN ANSI_BOLD "0x" ANSI_BRIGHT_GREEN "%04lx" ANSI_RESET
				"    " ANSI_YELLOW "%03hhu" ANSI_RESET "    "
				ANSI_GREEN "0x" ANSI_BRIGHT_GREEN "%02hhx" ANSI_RESET "\n",
				addr, byte, byte);
		}
	}

	else if (command[0] == "help" || command[0] == "h") {
		printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "start" ANSI_RESET);
		printf(ANSI_CYAN " [ ");
		printf(ANSI_GREEN "--stack-size <number of bytes>");
		printf(ANSI_CYAN " ]" ANSI_RESET);
		printf(" - ");
		printf(ANSI_YELLOW "starts the program" ANSI_RESET "\n");

		printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "step" ANSI_RESET);
		printf(" - ");
		printf(ANSI_YELLOW "executes the next instruction of the program" ANSI_RESET "\n");

		printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "run" ANSI_RESET);
		printf(" - ");
		printf(ANSI_YELLOW "runs the program until the next breakpoint" ANSI_RESET "\n");

		printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "list" ANSI_RESET);
		printf(ANSI_CYAN " [ ");
		printf(ANSI_GREEN "--top <number of instructions>");
		printf(ANSI_CYAN " ]" ANSI_RESET);
		printf(" - ");
		printf(ANSI_YELLOW "lists the next [top] instructions" ANSI_RESET "\n");

		printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "reg" ANSI_RESET);
		printf(" - ");
		printf(ANSI_YELLOW "lists the current state of the CPU registers" ANSI_RESET "\n");

		printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "lsbp" ANSI_RESET);
		printf(" - ");
		printf(ANSI_YELLOW "lists the addresses of the current set breakpoints" ANSI_RESET "\n");

		printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "bp" ANSI_RESET);
		printf(ANSI_GREEN " <address>" ANSI_RESET);
		printf(" - ");
		printf(ANSI_YELLOW "sets a breakpoint at address" ANSI_RESET "\n");

		printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "rmbp" ANSI_RESET);
		printf(ANSI_GREEN " <address>" ANSI_RESET);
		printf(" - ");
		printf(ANSI_YELLOW "unsets a breakpoint at address" ANSI_RESET "\n");

		printf(ANSI_BRIGHT_MAGENTA ANSI_ITALIC ANSI_BOLD "ds" ANSI_RESET);
		printf(ANSI_CYAN " [ ");
		printf(ANSI_GREEN "--top <number of bytes>");
		printf(ANSI_CYAN " ]" ANSI_RESET);
		printf(" - ");
		printf(ANSI_YELLOW "dumps the first [top] bytes on the stack" ANSI_RESET "\n");
	} else {
		printf(ANSI_RED "Unknown command \"%s\". Try \"help\"" ANSI_RESET "\n",
			command[0].c_str());
	}

	goto shell;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "Usage: ./debug program.teax\n");
		exit(1);
	}

	keypress::save_termios();

	char *file_path = argv[1];
	shell(file_path);
}