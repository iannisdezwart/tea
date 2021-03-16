#ifndef TEA_COMPILER_HEADER
#define TEA_COMPILER_HEADER

#include <bits/stdc++.h>

#include "util.hpp"
#include "tokeniser.hpp"
#include "parser.hpp"
#include "compiler-state.hpp"
#include "../Assembler/assembler.hpp"
#include "../Assembler/buffer.hpp"

using namespace std;

class Compiler {
	public:
		FILE *input_file;

		char *output_file_name;

		Assembler assembler;
		CompilerState compiler_state;

		Compiler(char *input_file_name, char *output_file_name)
			: output_file_name(output_file_name)
		{
			input_file = fopen(input_file_name, "r");

			if (input_file == NULL)
				err("Input file %s does not exist", input_file_name);
		}

		~Compiler()
		{
			fclose(input_file);
		}

		void compile()
		{
			Tokeniser tokeniser(input_file);
			vector<Token> tokens = tokeniser.tokenise();
			tokeniser.print_tokens();

			Parser parser(tokens);
			vector<ASTNode *> statements = parser.parse();
			parser.print_ast();

			// Parameter count

			assembler.push_64(0);
			assembler.call("main");
			assembler.jump("exit");

			for (size_t i = 0; i < statements.size(); i++) {
				statements[i]->compile(assembler, compiler_state);
			}

			assembler.add_label("exit");

			Buffer executable = assembler.assemble();
			executable.write_to_file(output_file_name);
		}
};

#endif