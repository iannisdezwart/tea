#include <cstdio>

struct FilePos
{
	size_t pos;
	size_t line;
	size_t col;

	FilePos()
		: pos(0), line(1), col(1) {}
};

/**
 * @brief Class that is used to read source files.
 * Allows peeking arbitrary bytes from the file.
 */
struct FileStreamReader
{
	// The file to read from.
	FILE *file;

	// The current position in the file.
	FilePos pos;

	/**
	 * @brief Constructs a new File Stream Reader object.
	 * @param file The file to read from.
	 */
	FileStreamReader(FILE *file)
		: file(file), pos() {}

	/**
	 * @brief Reads a character and returns the position before reading.
	 */
	FilePos
	read_char(int &c)
	{
		FilePos p = pos;
		c         = fgetc(file);
		pos.pos++;

		if (c == '\n')
		{
			pos.line++;
			pos.col = 1;
		}
		else
		{
			pos.col++;
		}

		return p;
	}

	/**
	 * Restores the file position to the given position.
	 */
	void
	restore(FilePos p)
	{
		pos = p;
		fseek(file, p.pos, SEEK_SET);
	}
};