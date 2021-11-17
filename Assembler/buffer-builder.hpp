#ifndef TEA_WRITE_BUFFER_HEADER
#define TEA_WRITE_BUFFER_HEADER

#include <bits/stdc++.h>

#include "buffer.hpp"

using namespace std;

/**
 * @brief A class that builds a buffer from a stream of data.
 * The data must be passed to the builder using the `push()` method.
 * The buffer grows automatically.
 */
class BufferBuilder {
	private:
		uint8_t *buffer;
		size_t capacity;

		/**
		 * @brief Returns the smallest power of two that
		 * is bigger than el_count.
		 * Used to compute the next buffer size.
		 */
		size_t fits(size_t el_count)
		{
			// Find the smallest power of two that is bigger
			// than el_count.
			// We do this by starting `size` at 2^64 and halving it
			// until it barely fits.

			for (uint8_t i = sizeof(size_t) * 8; i != 1; i--) {
				size_t size = 1 << (i - 1);
				if (size < el_count) return 1 << i;
			}

			// If we get here, the el_count is 0.

			return 1;
		}

	public:
		// The offset to the next element to be pushed.
		size_t offset = 0;

		/**
		 * @brief Constructs a new Buffer Builder object.
		 * @param init_size The initial size of the buffer.
		 */
		BufferBuilder(size_t init_size = 1024)
		{
			capacity = init_size;
			buffer = new uint8_t[capacity];
		}

		/**
		 * @brief Frees the buffer.
		 */
		void free_buffer()
		{
			delete[] buffer;
		}

		/**
		 * @return A constant pointer to the buffer.
		 */
		const uint8_t *data() const
		{
			return buffer;
		}

		/**
		 * @brief Reserves space for `size` more bytes.
		 * @param size The number of extra bytes to reserve.
		 */
		void reserve(size_t size)
		{
			// If the buffer is big enough, don't do anything.

			if (offset + size <= capacity) {
				return;
			}

			// Grow the buffer.

			size_t new_capacity = fits(offset + size);
			uint8_t *new_buffer = new uint8_t[new_capacity];

			memcpy(new_buffer, buffer, offset);

			delete[] buffer;
			buffer = new_buffer;
			capacity = new_capacity;
		}

		/**
		 * @brief Pushes bytes onto the buffer.
		 * @tparam intx_t The integer type of the data.
		 * @param value The value to push onto the buffer.
		 * @return The starting offset to the pushed data.
		 */
		template <typename intx_t>
		size_t push(intx_t value)
		{
			reserve(sizeof(intx_t));

			*((intx_t *) (buffer + offset)) = value;

			size_t prev_offset = offset;
			offset += sizeof(intx_t);

			return prev_offset;
		}

		/**
		 * @brief Pushes a string onto the buffer.
		 * @param str A std::string instance to push.
		 */
		void push_null_terminated_string(const string& str)
		{
			reserve(str.size() + 1);
			for (char c : str) push(c);
			push('\0');
		}

		/**
		 * @brief Returns a byte on the buffer.
		 * @param index The index of the byte to access.
		 * @returns The byte at the index.
		 */
		uint8_t operator[](size_t index) const
		{
			return buffer[index];
		}

		/**
		 * @brief Builds the buffer contents to a `Buffer` instance.
		 * @returns The buffer.
		 */
		Buffer build()
		{
			return Buffer(buffer, offset);
		}
};

#endif