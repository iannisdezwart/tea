#ifndef TEA_DEBUGGER_COMMAND_HEADER
#define TEA_DEBUGGER_COMMAND_HEADER

#include "Shared/ansi.hpp"
#include "Debugger/keypress.hpp"
#include "Debugger/util.hpp"

void
print_shell_prompt(const char *file_path, uint8_t *addr)
{
	printf(ANSI_CYAN ANSI_BOLD "%s" ANSI_RESET, file_path);

	if (addr != NULL)
	{
		printf(ANSI_YELLOW " [ ");
		printf(ANSI_GREEN ANSI_BOLD "0x" ANSI_BRIGHT_GREEN "%04llx" ANSI_RESET,
			(uint64_t) addr);
		printf(ANSI_YELLOW " ]");
	}

	printf(ANSI_BRIGHT_MAGENTA ANSI_BOLD " ➜ " ANSI_RESET);
}

/**
 * @brief Holds a history of commands typed by the user.
 * Used to scroll through the history of commands using the up and down
 * arrow keys.
 */
struct CommandHistory
{
	std::vector<std::string> history;
	size_t i = 0;

	void
	push(const std::string &line)
	{
		// Remove trailing empty lines.

		for (size_t i = history.size(); i != 0; i--)
		{
			if (history[i - 1] == "")
			{
				history.pop_back();
			}
			else
			{
				break;
			}
		}

		// Push the new line.

		history.push_back(line);
	}

	size_t
	size()
	{
		return history.size();
	}

	bool
	at_top()
	{
		return i == 0;
	}

	bool
	at_bottom()
	{
		return i == history.size() - 1;
	}

	void
	scroll_up()
	{
		if (i != 0)
		{
			i--;
		}
	}

	void
	scroll_down()
	{
		if (i != history.size() - 1)
		{
			i++;
		}
	}

	void
	scroll_to_bottom()
	{
		i = history.size() - 1;
	}

	const std::string &
	get()
	{
		return history[i];
	}

	void
	update_bottom(const std::string &new_line)
	{
		history[history.size() - 1] = new_line;
	}
};

CommandHistory command_history;

struct Command
{
	const std::string &command_line;

	// Basically split the line into words.

	void
	split_command_line()
	{
		size_t start;
		bool parsing = false;
		char quotes  = '\0';

		for (size_t i = 0; i < command_line.size(); i++)
		{
			if (!parsing)
			{
				// Ignore whitespace.

				if (is_whitespace(command_line[i]))
				{
					continue;
				}

				// Note down that quotes were used.

				if (command_line[i] == '"')
				{
					quotes = '"';
				}

				// Start parsing the next word.

				parsing = true;
				start   = i;
			}
			else
			{
				// Stop parsing the word if it ends and push the word.

				if ((command_line[i] == ' ' && !quotes)
					|| (quotes && command_line[i] == quotes))
				{
					parsing = false;
					args.push_back(command_line.substr(start, i - start));
				}
			}
		}

		// Push the trailing word if we did not stop parsing

		if (parsing)
		{
			args.push_back(command_line.substr(start));
		}
	}

	std::vector<std::string> args;

	Command(const std::string &command_line)
		: command_line(command_line)
	{
		split_command_line();
	}

	bool
	has_single_letter_flag(char flag)
	{
		for (size_t i = 0; i < args.size(); i++)
		{
			if (args[i].size() >= 2
				&& args[i][0] == '-'
				&& args[i][1] != '-')
			{
				for (size_t j = 0; j < args[i].size(); j++)
				{
					if (args[i][j] == flag)
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	bool
	has_word_flag(const std::string &flag)
	{
		for (size_t i = 0; i < args.size(); i++)
		{
			if (args[i].size() >= 3
				&& args[i][0] == '-'
				&& args[i][1] == '-')
			{
				std::string arg = args[i].substr(2);
				if (arg == flag)
				{
					return true;
				}
			}
		}

		return false;
	}

	bool
	has_flag(const std::string &flag)
	{
		// Single letter flag (e.g. "-f").

		if (flag.size() == 1)
		{
			return has_single_letter_flag(flag[0]);
		}

		// Word flag (e.g. "--force").

		else
		{
			return has_word_flag(flag);
		}

		return false;
	}

	std::string
	get_flag_value(const std::string &flag)
	{
		// Returns the value of a word flag (e.g. "--flag 123" -> "123").

		for (size_t i = 0; i < args.size(); i++)
		{
			if (args[i].size() >= 3
				&& args[i][0] == '-'
				&& args[i][1] == '-')
			{
				std::string arg = args[i].substr(2);
				if (arg == flag)
				{
					if (i == args.size() - 1)
					{
						return "";
					}

					return args[i + 1];
				}
			}
		}

		return "";
	}

	const std::string &
	operator[](size_t i)
	{
		return args[i];
	}

	size_t
	num_of_args()
	{
		return args.size();
	}

	static Command
	read(const char *file_path, uint8_t *addr)
	{
		// Reads the next command from the user.

		std::string line;
		size_t index = 0;

		// Show an empty line and add it to the command history.

		command_history.push(line);
		command_history.scroll_to_bottom();
		keypress::start_getch_mode();

		while (true)
		{
			// Erase the line and print the shell prompt + the user input line.

			printf(ANSI_ERASE_LINE(2) ANSI_CURSOR_TO_COL(1));
			print_shell_prompt(file_path, addr);
			printf("%s", line.c_str());

			// Place the cursor at the selected letter.

			if (line.size() != 0 && index != line.size())
			{
				ansi_cursor_back(line.size() - index);
			}

			// Await the next key press.

			uint8_t key        = keypress::get_key();
			bool at_very_left  = index == 0;
			bool at_very_right = index == line.size();

			// Handle the key.

			switch (key)
			{
			case keypress::BACKSPACE:
				// Delete the previous letter.

				if (!at_very_left)
				{
					line.erase(index - 1, 1);
					index--;
					command_history.update_bottom(line);
				}

				break;

			case keypress::CTRL_BACKSPACE:
			case keypress::ALT_BACKSPACE:
				// Delete the previous word.

				if (!at_very_left)
				{
					size_t space_index = 0;
					bool found_word    = false;

					for (size_t i = index; i != 0; i--)
					{
						char c          = line[i - 1];
						bool whitespace = is_whitespace(c);

						if (whitespace && found_word)
						{
							space_index = i;
							break;
						}

						if (!whitespace)
						{
							found_word = true;
						}
					}

					size_t length = index - space_index;
					line.erase(space_index, length);
					index -= length;
					command_history.update_bottom(line);
				}

				break;

			case keypress::DELETE:
				// Delete the next letter.

				if (!at_very_right)
				{
					line.erase(index, 1);
					command_history.update_bottom(line);
				}

				break;

			case keypress::ARROW_LEFT:
				// Move the cursor left.

				if (!at_very_left)
				{
					index--;
				}
				break;

			case keypress::ARROW_RIGHT:
				// Move the cursor right.

				if (!at_very_right)
				{
					index++;
				}
				break;

			case keypress::ARROW_UP:
				// Go back in history.

				if (!command_history.at_top())
				{
					command_history.scroll_up();
					line  = command_history.get();
					index = line.size();
				}

				break;

			case keypress::ARROW_DOWN:
				// Go forward in history.

				if (!command_history.at_bottom())
				{
					command_history.scroll_down();
					line  = command_history.get();
					index = line.size();
				}

				break;

			case keypress::ENTER:
				// Run the command.

				printf("\n");
				goto run_command;

			default:
				// Add the letter to the user input.

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

#endif