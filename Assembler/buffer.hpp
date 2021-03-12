#ifndef TEA_BUFFER_HEADER
#define TEA_BUFFER_HEADER

#include <bits/stdc++.h>

using namespace std;

class Buffer {
	public:
		uint8_t *data;
		size_t size;

		Buffer(uint8_t *data, size_t size) : data(data), size(size) {}

		~Buffer()
		{
			delete[] data;
		}

		template <typename intx_t>
		intx_t get(uint64_t offset)
		{
			return data[offset];
		}

		template <typename intx_t>
		void set(uint64_t offset, intx_t value)
		{
			data[offset] = value;
		}

		void write_to_file(const char *file_path)
		{
			FILE *file = fopen(file_path, "w");
			fwrite(data, 1, size, file);
			fclose(file);
		}

		static Buffer from_file(const char *file_path)
		{
			FILE *file = fopen(file_path, "r");

			fseek(file, 0, SEEK_END);
			size_t size = ftell(file);
			fseek(file, 0, SEEK_SET);

			uint8_t *buffer = new uint8_t[size];

			fread(buffer, 1, size, file);
			fclose(file);

			return Buffer(buffer, size);
		}

		static Buffer alloc(size_t size)
		{
			uint8_t *buffer = new uint8_t[size];
			return Buffer(buffer, size);
		}
};

#endif