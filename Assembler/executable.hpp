#ifndef TEA_EXECUTABLE_HEADER
#define TEA_EXECUTABLE_HEADER

#include <bits/stdc++.h>

#include "buffer.hpp"

/**
 * @brief Class that represents an executable.
 * Inherits from Buffer. It is used to store the executable.
 */
class Executable : public Buffer {
	public:
		// The size of the static data segment.
		uint64_t static_data_size;

		// The size of the program segment.
		uint64_t program_size;

		/**
		 * @brief Constructs a new `Executable` object.
		 * @param buffer A pointer to the buffer that contains the executable.
		 * @param size The size of the buffer that contains the executable.
		 * @param static_data_size The size of the static data segment.
		 * @param program_size The size of the program segment.
		 */
		Executable(uint8_t *buffer, size_t size,
			uint64_t static_data_size, uint64_t program_size)
			: Buffer(buffer, size),
				static_data_size(static_data_size),
				program_size(program_size) {}

		/**
		 * @brief Constructs an `Executable` object from a file.
		 * @param file_name The file name of the executable. Must exist.
		 * @returns An `Executable` object of the file.
		 */
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