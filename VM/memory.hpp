#ifndef TEA_VM_MEMORY_HEADER
#define TEA_VM_MEMORY_HEADER

#include <bits/stdc++.h>

namespace memory {
	/**
	 * @brief Allocates a memory block.
	 * @param size The size of the block.
	 * @returns A pointer to a block of memory.
	 */
	uint8_t *allocate(size_t size)
	{
		return new uint8_t[size];
	}

	/**
	 * @brief Tries to read from memory of the host machine.
	 * TODO: Support catching segmentation faults.
	 * @tparam The size of the data to read.
	 * @param addr The address to read from.
	 * @returns The read data.
	 */
	template <typename intx_t>
	intx_t get(uint8_t *addr)
	{
		return *(intx_t *) addr;
	}

	/**
	 * @brief Tries to write to memory of the host machine.
	 * TODO: Support catching segmentation faults.
	 * @tparam The size of the data to read.
	 * @param addr The address to read from.
	 * @param val The value to write.
	 */
	template <typename intx_t>
	void set(uint8_t *addr, intx_t val)
	{
		*(intx_t *) addr = val;
	}

	/**
	 * @brief Dumps a block of memory to display in hex.
	 * @param mem A pointer to the starting address of the block of memory.
	 * @param size The size of the block of memory.
	 * @param highlight_fg An index to the memory byte to highlight with
	 * red foreground colour. -1 indicates no highlighting.
	 * @param highlight_bg An index to the memory byte to highlight with
	 * yellow background colour. -1 indicates no highlighting.
	 */
	void dump(uint8_t *mem, size_t size, uint64_t highlight_fg = -1,
		uint64_t highlight_bg = -1)
	{
		for (size_t i = 0; i < size; i++) {
			if (highlight_fg == i) printf("\x1b[31m");
			if (highlight_bg == i) printf("\x1b[43m");

			uint8_t byte = mem[i];

			printf("0x%04lx    0x%02hhx    %03hhu\x1b[m\n", i, byte, byte);
		}

		printf("\n");
	}

	/**
	 * @brief Class used to easily read a block of memory sequentially.
	 */
	struct Reader {
			// A pointer to the memory byte to read from.
			uint8_t *addr;

			/**
			 * @brief Constructs a new Reader object.
			 * @param addr The starting address of the memory block.
			 */
			Reader(uint8_t *addr) : addr(addr) {}

			/**
			 * @brief Reads the next bytes from the memory block.
			 * @tparam intx_t The size of the data to read.
			 * @returns The read data.
			 */
			template <typename intx_t>
			intx_t read()
			{
				intx_t value = get<intx_t>(addr);
				addr += sizeof(intx_t);
				return value;
			}
	};
};

#endif