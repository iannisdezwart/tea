#ifndef TEA_COMPILER_HEADER
#define TEA_COMPILER_HEADER

#include <bits/stdc++.h>

#include "util.hpp"
#include "tokeniser.hpp"
#include "parser.hpp"

using namespace std;

void compile(char *input_file_name, char *output_file_name)
{
	FILE *input_file = fopen(input_file_name, "r");
	FILE *output_file = fopen(output_file_name, "w");

	if (input_file == NULL)
		err("Input file %s does not exist", input_file_name);

	if (output_file == NULL)
		err("Output file %s does not exist", output_file_name);

	Tokeniser tokeniser(input_file);
	vector<Token> tokens = tokeniser.tokenise();

	printf("Number of tokens: %ld\n\n", tokens.size());

	for (Token token : tokens) {
		printf("Token: { type: %d, value: \"%s\", line: %ld, col: %ld }\n",
			token.type, token.value.c_str(), token.line, token.col);
	}

	Parser parser(tokens);
	parser.parse();
	parser.print_ast();

	fclose(input_file);
	fclose(output_file);
}

#endif