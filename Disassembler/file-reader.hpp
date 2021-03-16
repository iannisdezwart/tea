#ifndef TEA_FILE_READER_HEADER
#define TEA_FILE_READER_HEADER

#include <bits/stdc++.h>

using namespace std;

class FileReader {
	public:
		FILE *file;
		size_t read_bytes = 0;

		FileReader(FILE *file) : file(file) {}

		template <typename intx_t>
		intx_t read()
		{
			intx_t value = 0;

			// Little endian

			for (uint8_t i = 0; i < sizeof(intx_t); i++) {
				int c = fgetc(file);

				if (c == EOF) return EOF;

				intx_t byte = c << (i * 8);
				value |= byte;
			}

			read_bytes += sizeof(intx_t);
			return value;
		}
};

#endif