#include <bits/stdc++.h>

/**
 * @brief Class that is used to read source files.
 * Allows peeking arbitrary bytes from the file.
 */
struct FileStreamReader
{
	// The file to read from.
	FILE *file;

	// The current position in the file.
	// Peeking does not change this.
	size_t pos = 0;

	// The current line number.
	size_t cur_line = 1;

	// The current column number.
	size_t cur_col = 1;

	// The old line number.
	size_t line = 1;

	// The old column number.
	size_t col = 1;

	/**
	 * @brief Constructs a new File Stream Reader object.
	 * @param file The file to read from.
	 */
	FileStreamReader(FILE *file) : file(file) {}

	/**
	 * @brief Reads the previous byte from the file.
	 * Does not change the current position.
	 * @returns The previous byte.
	 */
	int
	prev_byte()
	{
		fseek(file, -1, SEEK_CUR);
		return fgetc(file);
	}

	/**
	 * @brief Reads a single byte from the file.
	 * @returns The read byte.
	 */
	int
	read_byte()
	{
		int byte = peek_byte();
		advance();
		return byte;
	}

	/**
	 * @brief Peeks a single byte from the file.
	 * Reads the byte, but does not advance the file pointer.
	 * @returns The peeked byte.
	 */
	int
	peek_byte()
	{
		int byte = fgetc(file);

		if (prev_byte() == '\n')
		{
			cur_line++;
			cur_col = 1;
		}
		else
		{
			cur_col++;
		}

		return byte;
	}

	/**
	 * @brief Returns the file pointer to its original position
	 * before peeking.
	 */
	void
	reset()
	{
		cur_line = line;
		cur_col = col;
		fseek(file, pos, SEEK_SET);
	}

	/**
	 * @brief Advances the file pointer to after the peeked bytes.
	 */
	void
	advance()
	{
		pos = ftell(file);
		line = cur_line;
		col = cur_col;
	}
};