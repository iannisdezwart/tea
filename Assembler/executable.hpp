#ifndef TEA_EXECUTABLE_HEADER
#define TEA_EXECUTABLE_HEADER

#include <bits/stdc++.h>

#include "buffer.hpp"

using namespace std;

class Executable : public Buffer {
	public:
		uint64_t static_data_size;
		uint64_t program_size;

		Executable(uint8_t *buffer, size_t size,
			uint64_t static_data_size, uint64_t program_size)
			: Buffer(buffer, size),
				static_data_size(static_data_size),
				program_size(program_size) {}

		static Executable from_file(const char *file_name)
		{
			Buffer buffer = Buffer::from_file(file_name);
			size_t static_data_size = buffer.get<uint64_t>(0);
			size_t program_size = buffer.get<uint64_t>(8);
			size_t size_of_executable = buffer.size - 16;
			uint8_t *executable = new uint8_t[size_of_executable];
			memcpy(executable, buffer.data + 16, size_of_executable);

			return Executable(executable, size_of_executable,
				static_data_size, program_size);
		}
};

#endif