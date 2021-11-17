#ifndef TEA_RAM_DEVICE_HEADER
#define TEA_RAM_DEVICE_HEADER

#include <bits/stdc++.h>

#include "memory-device.hpp"
#include "../Assembler/buffer.hpp"

using namespace std;

/**
 * @brief This class is not currently in use.
 * It was part of the memory mapper, but I removed it from the CPU class,
 * since it slowed down things considerably.
 * Most of the methods implemented were reimplemented in the `memory.hpp`
 * file, which is kind of similar to the RamDevice, except it works with
 * raw memory directly, thus increasing performance dramatically.
 */
class RamDevice : public MemoryDevice, public Buffer {
	public:
		RamDevice(uint64_t from, uint64_t to)
			: MemoryDevice(from, to),
				Buffer(Buffer::alloc(to - from)) {}

		const char *type()
		{
			return "RamDevice";
		}

		void copy_from(uint8_t *source, size_t source_size)
		{
			memcpy(data, source, source_size);
		}

		uint64_t size()
		{
			return to - from;
		}

		template <typename intx_t>
		intx_t get(uint64_t offset)
		{
			return *((intx_t *) (data + offset));
		}

		template <typename intx_t>
		void set(uint64_t offset, intx_t value)
		{
			*((intx_t *) (data + offset)) = value;
		}

		void dump(
			uint64_t left_bound,
			uint64_t right_bound,
			uint64_t highlight_fg = -1,
			uint64_t highlight_bg = -1
		) {
			for (uint64_t i = left_bound; i <= right_bound; i++) {
				if (highlight_fg == i) printf("\x1b[31m");
				if (highlight_bg == i) printf("\x1b[43m");
				uint8_t byte = get<uint8_t>(i - from);
				printf("0x%04lx    0x%02hhx    %03hhu\x1b[m\n", i, byte, byte);
			}

			printf("\n");
		}
};

#endif