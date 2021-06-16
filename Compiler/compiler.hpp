#ifndef TEA_COMPILER_HEADER
#define TEA_COMPILER_HEADER

#include <bits/stdc++.h>

#include "util.hpp"
#include "tokeniser.hpp"
#include "parser.hpp"
#include "compiler-state.hpp"
#include "debugger-symbols.hpp"
#include "../Assembler/assembler.hpp"
#include "../Assembler/buffer.hpp"
#include "ASTNodes/VariableDeclaration.hpp"

using namespace std;

class Compiler {
	public:
		FILE *input_file;

		char *output_file_name;

		Assembler assembler;
		CompilerState compiler_state;

		Compiler(char *input_file_name, char *output_file_name, bool debug)
			: output_file_name(output_file_name), compiler_state(debug)
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

			// Collect declarations

			vector<VariableDeclaration *> global_var_decls;
			vector<FunctionDeclaration *> fn_decls;
			vector<ClassDeclaration *> class_decls;

			for (size_t i = 0; i < statements.size(); i++) {
				ASTNode *statement = statements[i];

				switch (statement->type) {
					case VARIABLE_DECLARATION:
						global_var_decls.push_back((VariableDeclaration *) statement);
						break;

					case FUNCTION_DECLARATION:
						fn_decls.push_back((FunctionDeclaration *) statement);
						break;

					case CLASS_DECLARATION:
					{
						ClassDeclaration *class_decl = (ClassDeclaration *) statement;
						class_decls.push_back(class_decl);
						break;
					}

					default:
						err_at_token(statement->accountable_token, "Unexpected statement",
							"Unexpected statement of type %d",
							statement->type);
						break;
				}
			}

			// Add classes

			for (size_t i = 0; i < class_decls.size(); i++) {
				ClassDeclaration *class_decl = class_decls[i];
				size_t byte_size = class_decl->byte_size(compiler_state);
				Class cl(byte_size);

				for (TypeIdentifierPair *field : class_decl->fields) {
					string name = field->identifier_token.value;
					Type type = field->get_type(compiler_state);
					cl.add_field(name, type);
				}

				compiler_state.add_class(class_decl->class_name, cl);
			}

			// Add global variables

			for (size_t i = 0; i < global_var_decls.size(); i++) {
				VariableDeclaration *decl = global_var_decls[i];
				string var_name = decl->type_and_id_pair->get_identifier_name();
				Type var_type = decl->get_type(compiler_state);

				compiler_state.add_global(var_name, var_type);
			}

			// Allocate space for globals & update stack and frame pointer

			assembler.allocate_stack(compiler_state.globals_size);
			assembler.add_64_into_reg(compiler_state.globals_size, R_STACK_P_ID);
			assembler.add_64_into_reg(compiler_state.globals_size, R_FRAME_P_ID);

			// Compile intitialisation values for global variables

			for (size_t i = 0; i < global_var_decls.size(); i++) {
				VariableDeclaration *decl = global_var_decls[i];
				decl->compile(assembler, compiler_state);
			}

			// Push parameter count and call main

			assembler.push_64(0);
			assembler.call("main");
			assembler.jump("exit");

			// Compile function declarations

			for (size_t i = 0; i < fn_decls.size(); i++) {
				FunctionDeclaration *decl = fn_decls[i];
				decl->compile(assembler, compiler_state);
			}

			// Compile class method declarations

			for (size_t i = 0; i < class_decls.size(); i++) {
				ClassDeclaration *decl = class_decls[i];
				decl->compile(assembler, compiler_state);
			}

			assembler.add_label("exit");

			Buffer executable = assembler.assemble();
			executable.write_to_file(output_file_name);

			// Create debugger symbols

			if (compiler_state.debug) {
				compiler_state.debugger_symbols.build(output_file_name);
			}
		}
};

#endif