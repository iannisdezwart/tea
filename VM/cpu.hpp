#ifndef TEA_CPU_HEADER
#define TEA_CPU_HEADER

#include <bits/stdc++.h>

#include "memory.hpp"
#include "../Compiler/byte_code.hpp"

using namespace std;

class CPU {
	public:
		Memory program;
		Memory stack;

		uint64_t r_instruction_p = 0;
		uint64_t r_stack_p = 0;
		uint64_t r_frame_p = 0;

		uint64_t r_accumulator = 0;

		uint64_t r_1 = 0;
		uint64_t r_2 = 0;
		uint64_t r_3 = 0;
		uint64_t r_4 = 0;

		CPU(vector<uint8_t>& program)
			: program(program.size()), stack(20) {}

		void run()
		{
			while (r_instruction_p < program.size) {
				uint64_t instruction = fetch<uint64_t>();
				execute(instruction);
				dump_stack();
			}
		}

		void dump_stack()
		{
			printf("Stack dump:\n");

			for (size_t i = 0; i < stack.size; i++) {
				printf("%03d ", (int) stack.get<uint8_t>(i));
			}

			printf("\n");
		}

		template <typename intx_t>
		intx_t fetch()
		{
			intx_t instruction = program.get<intx_t>(r_instruction_p);
			printf("fetch, r_instruction_p = %ld\n", r_instruction_p);
			r_instruction_p += sizeof(intx_t);
			printf("fetch, r_instruction_p = %ld\n", r_instruction_p);
			return instruction;
		}

		void execute(uint64_t instruction)
		{
			switch (instruction) {
				case PUSH_8:
				{
					printf("PUSH_8, r_stack_p = %ld\n", r_stack_p);
					uint8_t a = fetch<uint8_t>();
					stack.set(r_stack_p, a);
					r_stack_p += 1;
					printf("PUSH_8, r_stack_p = %ld, a = %hhu\n", r_stack_p, a);
					break;
				}

				case PUSH_16:
					stack.set(r_stack_p, fetch<uint16_t>());
					r_stack_p += 2;
					break;

				case PUSH_32:
					stack.set(r_stack_p, fetch<uint32_t>());
					r_stack_p += 4;
					break;

				case PUSH_64:
					stack.set(r_stack_p, fetch<uint64_t>());
					r_stack_p += 8;
					break;

				case POP_8:
					r_1 = stack.get<uint8_t>(r_stack_p);
					r_stack_p -= 1;
					break;

				case POP_16:
					r_1 = stack.get<uint16_t>(r_stack_p);
					r_stack_p -= 2;
					break;

				case POP_32:
					r_1 = stack.get<uint32_t>(r_stack_p);
					r_stack_p -= 4;
					break;

				case POP_64:
					r_1 = stack.get<uint64_t>(r_stack_p);
					r_stack_p -= 8;
					break;
			}
		}
};

#endif