#ifndef TEA_BUFFER_HEADER
#define TEA_BUFFER_HEADER

#include <bits/stdc++.h>

class Buffer {
	public:
		// A pointer to the data on the buffer.
		uint8_t *data;

		// The size of the buffer.
		size_t size;

		/**
		 * @brief Constructs a new Buffer object.
		 * @param data A pointer to the data for the buffer.
		 * @param size The size of the buffer.
		 */
		Buffer(uint8_t *data, size_t size) : data(data), size(size) {}

		/**
		 * @brief Destroys the Buffer object.
		 * Frees the underlying buffer.
		 */
		~Buffer()
		{
			delete[] data;
		}

		/**
		 * @brief Gets data off the buffer.
		 * @tparam intx_t The type of data.
		 * @param offset The offset to the data.
		 * @returns A copy of the data at the offset.
		 */
		template <typename intx_t>
		intx_t get(uint64_t offset)
		{
			return *((intx_t *) (data + offset));
		}

		/**
		 * @brief Sets data to the buffer.
		 * @tparam intx_t The type of data.
		 * @param offset The offset to the data.
		 * @param value The value to store on the buffer.
		 */
		template <typename intx_t>
		void set(uint64_t offset, intx_t value)
		{
			*((intx_t *) (data + offset)) = value;
		}

		/**
		 * @brief Writes the buffer to a file.
		 * @param file_path The file path of the file to write to.
		 * If the file does not exist, a file is created.
		 * If the file does exist, it is overwritten.
		 */
		void write_to_file(const char *file_path)
		{
			FILE *file = fopen(file_path, "w");
			fwrite(data, 1, size, file);
			fclose(file);
		}

		/**
		 * @brief Constructs a `Buffer` from a file.
		 * @param file_path The path to the file to read. Must exist.
		 * @returns A buffer containing the data from the file.
		 */
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

		/**
		 * @brief Allocates an empty buffer of a certain size.
		 * The data is uninitialised, watch out for UB.
		 * @param size The size to allocate.
		 * @returns The allocated buffer.
		 */
		static Buffer alloc(size_t size)
		{
			uint8_t *buffer = new uint8_t[size];
			return Buffer(buffer, size);
		}
};

#endif