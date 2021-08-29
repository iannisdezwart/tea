#ifndef TEA_VM_MEMORY_HEADER
#define TEA_VM_MEMORY_HEADER

#include <bits/stdc++.h>

namespace memory {
	uint8_t *allocate(size_t size)
	{
		return new uint8_t[size];
	}

	template <typename intx_t>
	intx_t get(uint8_t *addr)
	{
		return *(intx_t *) addr;
	}

	template <typename intx_t>
	void set(uint8_t *addr, intx_t val)
	{
		*(intx_t *) addr = val;
	}

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

	class Reader {
		public:
			uint8_t *addr;

			Reader(uint8_t *addr) : addr(addr) {}

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