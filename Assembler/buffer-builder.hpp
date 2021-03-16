#ifndef TEA_WRITE_BUFFER_HEADER
#define TEA_WRITE_BUFFER_HEADER

#include <bits/stdc++.h>

#include "buffer.hpp"

using namespace std;

class BufferBuilder {
	private:
		uint8_t *buffer;
		size_t capacity;

		size_t fits(size_t el_count)
		{
			for (uint8_t i = sizeof(size_t) * 8; i != 1; i--) {
				size_t size = 1 << (i - 1);
				if (size != 0) return size;
			}

			return 1;
		}

	public:
		size_t offset = 0;

		BufferBuilder(size_t init_size = 1024)
		{
			capacity = init_size;
			buffer = new uint8_t[capacity];
		}

		void free_buffer()
		{
			delete[] buffer;
		}

		const uint8_t *data() const
		{
			return buffer;
		}

		void reserve(size_t size)
		{
			if (offset + size > capacity) {
				size_t new_capacity = fits(offset + size);
				uint8_t *new_buffer = new uint8_t[new_capacity];

				memcpy(new_buffer, buffer, offset);

				delete[] buffer;
				buffer = new_buffer;
			}
		}

		template <typename intx_t>
		size_t push(intx_t value)
		{
			reserve(sizeof(intx_t));

			*((intx_t *) (buffer + offset)) = value;

			size_t prev_offset = offset;
			offset += sizeof(intx_t);

			return prev_offset;
		}

		// const uint8_t& operator[](size_t index) const
		// {
		// 	return buffer[index];
		// }

		uint8_t operator[](size_t index) const
		{
			return buffer[index];
		}

		Buffer build()
		{
			return Buffer(buffer, offset);
		}
};

#endif