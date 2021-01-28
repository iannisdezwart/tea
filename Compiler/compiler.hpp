#ifndef TEA_COMPILER_HEADER
#define TEA_COMPILER_HEADER

#include <bits/stdc++.h>

#include "util.hpp"
#include "tokeniser.hpp"
#include "parser.hpp"

using namespace std;

class Compiler {
	public:
		FILE *input_file;
		FILE *output_file;

		unordered_map<string, vector<char>> constants;
		unordered_map<string, size_t> globals;
		unordered_map<string, vector<char>> functions;

		Compiler(char *input_file_name, char *output_file_name)
		{
			input_file = fopen(input_file_name, "r");
			output_file = fopen(output_file_name, "w");

			if (input_file == NULL)
				err("Input file %s does not exist", input_file_name);

			if (output_file == NULL)
				err("Output file %s does not exist", output_file_name);
		}

		~Compiler()
		{
			fclose(input_file);
			fclose(output_file);
		}

		void compile()
		{
			Tokeniser tokeniser(input_file);
			vector<Token> tokens = tokeniser.tokenise();
			tokeniser.print_tokens();

			Parser parser(tokens);
			vector<ASTNode *> statements = parser.parse();
			parser.print_ast();

			for (size_t i = 0; i < statements.size(); i++) {
				statements[i]->compile(constants, globals, functions);
			}
		}
};

#endif