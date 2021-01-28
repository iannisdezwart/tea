#ifndef TEA_VM_HEADER
#define TEA_VM_HEADER

#include <bits/stdc++.h>
#include "cpu.hpp"
#include "memory.hpp"
#include "../Compiler/byte_code.hpp"

using namespace std;

int main()
{
	// Memory memory(100);

	// memory.set<uint32_t>(0, 0x12345678);
	// uint32_t m0 = memory.get<uint32_t>(0);
	// uint8_t m0 = memory.get<uint8_t>(0);
	// uint8_t m1 = memory.get<uint8_t>(1);
	// uint8_t m2 = memory.get<uint8_t>(2);
	// uint8_t m3 = memory.get<uint8_t>(3);

	// printf("m0 = %hhx\n", m0);
	// printf("m1 = %hhx\n", m1);
	// printf("m2 = %hhx\n", m2);
	// printf("m3 = %hhx\n", m3);

	// uint32_t m = memory.get<uint32_t>(0);

	// printf("m = %lx\n", m);

	vector<uint8_t> instructions;
	instructions.push_back(0);
	instructions.push_back(0);
	instructions.push_back(0);
	instructions.push_back(0);
	instructions.push_back(0);
	instructions.push_back(0);
	instructions.push_back(0);
	instructions.push_back(PUSH_8);
	instructions.push_back(123);
	instructions.push_back(0);
	instructions.push_back(0);
	instructions.push_back(0);
	instructions.push_back(0);
	instructions.push_back(0);
	instructions.push_back(0);
	instructions.push_back(0);
	instructions.push_back(POP_8);

	CPU cpu(instructions);
	cpu.run();

	printf("r_1 = %lu\n", cpu.r_1);
}

#endif