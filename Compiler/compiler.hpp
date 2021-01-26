#ifndef TEA_COMPILER_HEADER
#define TEA_COMPILER_HEADER

#include <bits/stdc++.h>

#include "util.hpp"

using namespace std;

void compile(char *input_file_name, char *output_file_name)
{
	FILE *input_file = fopen(input_file_name, "r");
	FILE *output_file = fopen(output_file_name, "w");

	if (input_file == NULL)
		err("Input file %s does not exist", input_file_name);

	if (output_file == NULL)
		err("Output file %s does not exist", output_file_name);

	vector<>
}

#endif