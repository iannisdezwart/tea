#ifndef TEA_MEMORY_HEADER
#define TEA_MEMORY_HEADER

#include <bits/stdc++.h>

#include "../Assembler/byte_code.hpp"

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

		Memory(uint8_t *init_mem, size_t init_mem_size, size_t trailing_size = 0)
		{
			size = init_mem_size + trailing_size;
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

		void check_memory(uint64_t offset)
		{
			if (offset >= size) {
				printf("Segmentation Fault\n");
				printf("VM prevented access to memory at %ld (0x%lx)\n",
					(size_t) offset, (size_t) offset);
				printf("Only the region [ 0x0, 0x%lx ) is allowed\n",
					(uint64_t) size);
				abort();
			}
		}

		template <typename intx_t>
		intx_t get(uint64_t offset)
		{
			check_memory(offset);
			return memory[offset];
		}

		template <typename intx_t>
		void set(uint64_t offset, intx_t value)
		{
			check_memory(offset);
			memory[offset] = value;
		}

		void dump(
			uint64_t left_bound,
			uint64_t right_bound,
			uint64_t highlight_fg = -1,
			uint64_t highlight_bg = -1
		) {
			for (uint64_t i = left_bound; i < right_bound; i++) {
				if (highlight_fg == i) printf("\x1b[31m");
				if (highlight_bg == i) printf("\x1b[43m");
				uint8_t byte = get<uint8_t>(i);
				printf("0x%04lx    0x%02hhx    %03hhu\x1b[m\n", i, byte, byte);
			}

			printf("\n");
		}

		void write_to_file(const char *file_path)
		{
			FILE *file = fopen(file_path, "w");
			fwrite(memory, 1, size, file);
			fclose(file);
		}

		static Memory from_file(const char *file_path)
		{
			FILE *file = fopen(file_path, "r");

			fseek(file, 0, SEEK_END);
			size_t file_size = ftell(file);
			fseek(file, 0, SEEK_SET);

			Memory memory(file_size);

			for (size_t i = 0; i < file_size; i++) {
				memory.set<uint8_t>(i, fgetc(file));
			}

			fclose(file);

			return memory;
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

		Memory build()
		{
			return Memory(buffer.data(), i);
		}
};

#endif