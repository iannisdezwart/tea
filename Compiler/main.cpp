#include <bits/stdc++.h>

#include "compiler.hpp"
#include "util.hpp"

using namespace std;

int main(int argc, char **argv)
{
	if (argc < 2) err("Usage: ./compile input.tea output");

	char *input_file_name = argv[0];
	char *output_file_name = argv[1];

	compile(input_file_name, output_file_name);
}