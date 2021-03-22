#include <bits/stdc++.h>

#include "disassembler.hpp"

using namespace std;

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "Usage: ./deassemble program.teax\n");
		exit(1);
	}

	char *file_in_name = argv[1];
	FILE *file_in = fopen(file_in_name, "r");

	Disassembler disassembler(file_in, stdout);
	disassembler.disassemble();

	fclose(file_in);
}