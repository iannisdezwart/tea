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

		Memory(uint8_t *init_mem, size_t init_mem_size)
		{
			size = init_mem_size;
			memory = new uint8_t[size];
			memcpy(memory, init_mem, init_mem_size);
		}

		~Memory()
		{
			delete[] memory;
		}

		uint8_t *location()
		{
			return memory;
		}

		void check_memory(uint8_t *location_p)
		{
			if (location_p < memory || location_p >= memory + size) {
				printf("Segmentation Fault\n");
				printf("VM prevented access to memory at %ld (0x%lx)\n",
					(size_t) location_p, (size_t) location_p);
				exit(139);
			}
		}

		template <typename intx_t>
		intx_t get(uint8_t *location_p)
		{
			check_memory(location_p);
			intx_t *value_p = (intx_t *) (location_p);
			return *value_p;
		}

		template <typename intx_t>
		void set(uint8_t *location_p, intx_t value)
		{
			check_memory(location_p);
			intx_t *value_p = (intx_t *) (location_p);
			*value_p = value;
		}

		void dump(
			uint8_t *left_bound,
			uint8_t *right_bound,
			uint8_t *highlight = NULL
		) {
			for (uint8_t *it = left_bound; it < right_bound; it++) {
				if (highlight == it) printf("\x1b[92m");
				printf("%03d\x1b[m ", (int) get<uint8_t>(it));
			}

			printf("\n");
		}
};

class MemoryBuilder {
	public:
		vector<uint8_t> buffer;
		size_t i = 0;

		MemoryBuilder(size_t init_size = 1024) : buffer(init_size) {}

		template <typename intx_t>
		size_t push(intx_t value)
		{
			buffer.reserve(sizeof(intx_t));

			intx_t *value_p = (intx_t *) (buffer.data() + i);
			*value_p = value;

			size_t prev_i = i;
			i += sizeof(intx_t);
			return prev_i;
		}

		Memory *build(size_t stack_size)
		{
			return new Memory(buffer.data(), i + stack_size);
		}
};

#endif