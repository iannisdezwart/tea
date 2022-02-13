#include <bits/stdc++.h>

#include "cpu.hpp"
#include "../Assembler/assembler.hpp"

#define STACK_SIZE 8 * 1024 * 1024 // 8MB

int
main(int argc, char **argv)
{
	if (argc < 2)
	{
		fprintf(stderr, "Usage: ./vm input_file_name.teax\n");
		exit(1);
	}

	const char *file_path = argv[1];

	Executable executable = Executable::from_file(file_path);
	CPU cpu(executable, STACK_SIZE);

	try
	{
		cpu.run();
		printf("VM exited with exit code %llu\n", cpu.regs[R_RET]);
	}
	catch (const std::string &err_message)
	{
		std::cout << err_message;
		abort();
	}
}