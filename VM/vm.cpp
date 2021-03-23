#include <bits/stdc++.h>

// #define CPU_DUMP_DEBUG

#include "cpu.hpp"
#include "../Assembler/assembler.hpp"

using namespace std;

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "Usage: ./vm input_file_name.teax\n");
		exit(1);
	}

	const char *file_path = argv[1];

	Executable executable = Executable::from_file(file_path);
	CPU cpu(executable, 200);
	cpu.memory_mapper.print();
	cpu.run();

	printf("VM exited with exit code %lu\n", cpu.r_accumulator_0);
}