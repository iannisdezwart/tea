#ifndef TEA_MEMORY_HEADER
#define TEA_MEMORY_HEADER

#include <bits/stdc++.h>

using namespace std;

class Memory {
	private:
		uint8_t *memory;

	public:
		size_t size;

		Memory(size_t mem_size)
		{
			size = mem_size;
			memory = new uint8_t[size];
		}

		~Memory()
		{
			delete[] memory;
		}

		template <typename intx_t>
		intx_t get(size_t offset)
		{
			intx_t value = 0;

			for (uint8_t i = 0; i < sizeof(intx_t); i++) {
				value |= memory[offset + i] << (8 * (sizeof(intx_t) - 1 - i));
			}

			return value;
		}

		template <typename intx_t>
		void set(size_t offset, intx_t value)
		{
			for (uint8_t i = 0; i < sizeof(intx_t); i++) {
				uint8_t byte = (value >> (8 * (sizeof(intx_t) - 1 - i))) & 0xFF;
				memory[offset + i] = byte;
			}
		}
};

#endif