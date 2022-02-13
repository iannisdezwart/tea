#ifndef TEA_FILE_READER_HEADER
#define TEA_FILE_READER_HEADER

#include <bits/stdc++.h>

/**
 * @brief Class that is used to read a file in chunks of user-defined size.
 */
struct FileReader {
		// The file to read from.
		FILE *file;

		// The number of read bytes so far.
		size_t read_bytes = 0;

		/**
		 * @brief Constructs a new File Reader object.
		 * @param file The file to read from.
		 */
		FileReader(FILE *file) : file(file) {}

		/**
		 * @brief Reads a chunk of data from the file.
		 * @tparam intx_t The type of the data to read.
		 * @returns The read chunk.
		 */
		template <typename intx_t>
		intx_t read()
		{
			intx_t value = 0;

			// Only works on little-endian systems.

			for (uint8_t i = 0; i < sizeof(intx_t); i++) {
				int c = fgetc(file);

				if (c == EOF) return EOF;

				intx_t byte = (intx_t) c << (i * 8);
				value |= byte;
			}

			read_bytes += sizeof(intx_t);
			return value;
		}
};

#endif