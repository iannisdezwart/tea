#include <bits/stdc++.h>

// #define CPU_DUMP_DEBUG

#include "cpu.hpp"
#include "memory.hpp"
#include "../Assembler/assembler.hpp"

using namespace std;

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage: ./vm input_file_name.teax\n");
		exit(1);
	}

	const char *file_path = argv[1];

	Executable executable = Executable::from_file(file_path);
	CPU cpu(executable, 100);
	cpu.run();
}