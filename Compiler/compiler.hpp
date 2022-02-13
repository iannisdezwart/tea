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

/**
 * @brief Class responsible for compiling a source file into a byte code file.
 * Takes a source file name and compiles this file it into a given output file.
 */
struct Compiler {
		// A file pointer to the source file.
		FILE *input_file;

		// The output file name.
		// Will contain the byte code after compilation.
		char *output_file_name;


		// The assembler object.
		Assembler assembler;

		// The compiler state object.
		CompilerState compiler_state;


		/**
		 * @brief Constructor.
		 * Opens the input file and initialises the compiler state.
		 * @param input_file_name The name of the source file to be compiled.
		 * @param output_file_name The name of the output file.
		 */
		Compiler(char *input_file_name, char *output_file_name, bool debug)
			: output_file_name(output_file_name), compiler_state(debug)
		{
			input_file = fopen(input_file_name, "r");

			if (input_file == NULL)
				err("Input file %s does not exist", input_file_name);
		}

		/**
		 * @brief Destructor.
		 * Closes the input file.
		 */
		~Compiler()
		{
			fclose(input_file);
		}

		/**
		 * @brief Compiles the source file into a byte code file.
		 * Tokenises the source file,
		 * parses the tokens and compiles them into byte code.
		 */
		void compile()
		{
			// Tokenise the source file.

			Tokeniser tokeniser(input_file);
			std::vector<Token> tokens = tokeniser.tokenise();
			tokeniser.print_tokens();

			// Parse the tokens.

			Parser parser(tokens);
			std::vector<ASTNode *> statements = parser.parse();
			parser.print_ast();

			// Collect declarations.

			std::vector<VariableDeclaration *> global_var_decls;
			std::vector<FunctionDeclaration *> fn_decls;
			std::vector<ClassDeclaration *> class_decls;

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
						class_decls.push_back((ClassDeclaration *) statement);
						break;
					}

					default:
						err_at_token(statement->accountable_token, "Unexpected statement",
							"Unexpected statement of type %d",
							statement->type);
						break;
				}
			}

			// Add classes.

			for (size_t i = 0; i < class_decls.size(); i++) {
				ClassDeclaration *class_decl = class_decls[i];
				size_t byte_size = class_decl->byte_size(compiler_state);
				Class cl(byte_size);

				for (TypeIdentifierPair *field : class_decl->fields) {
					const std::string& name = field->identifier_token.value;
					Type type = field->get_type(compiler_state);
					cl.add_field(name, type);
				}

				for (FunctionDeclaration *method : class_decl->methods) {
					const std::string& name = method->type_and_id_pair->identifier_token.value;
					Function method_type = method->get_fn_type(compiler_state);
					cl.add_method(name, method_type);
				}

				compiler_state.add_class(class_decl->class_name, cl);
			}

			// Add global variables.

			for (size_t i = 0; i < global_var_decls.size(); i++) {
				VariableDeclaration *decl = global_var_decls[i];
				std::string var_name = decl->type_and_id_pair->get_identifier_name();
				Type var_type = decl->get_type(compiler_state);

				compiler_state.add_global(var_name, var_type);
			}

			// Start assembling.
			// Allocate space for globals & update stack and frame pointer.

			assembler.allocate_stack(compiler_state.globals_size);
			assembler.add_64_into_reg(compiler_state.globals_size, R_STACK_PTR);
			assembler.add_64_into_reg(compiler_state.globals_size, R_FRAME_PTR);

			// Compile intitialisation values for global variables.

			for (size_t i = 0; i < global_var_decls.size(); i++) {
				VariableDeclaration *decl = global_var_decls[i];
				decl->compile(assembler, compiler_state);
			}

			// Push parameter count and call main.

			assembler.push_64(0);
			assembler.call("main");
			assembler.jump("exit");

			// Define functions.

			for (size_t i = 0; i < fn_decls.size(); i++) {
				FunctionDeclaration *decl = fn_decls[i];
				decl->define(compiler_state);
			}

			// Define class methods.

			for (size_t i = 0; i < class_decls.size(); i++) {
				ClassDeclaration *decl = class_decls[i];
				decl->define(compiler_state);
			}

			parser.print_ast();

			// Compile function declarations.

			for (size_t i = 0; i < fn_decls.size(); i++) {
				FunctionDeclaration *decl = fn_decls[i];
				decl->compile(assembler, compiler_state);
			}

			// Compile class method declarations.

			for (size_t i = 0; i < class_decls.size(); i++) {
				ClassDeclaration *decl = class_decls[i];
				decl->compile(assembler, compiler_state);
			}

			// Add an exit label.
			// Jumped to after the main function finishes.

			assembler.add_label("exit");

			// Write the byte code to the output file.

			Buffer executable = assembler.assemble();
			executable.write_to_file(output_file_name);

			// Create debugger symbols

			if (compiler_state.debug) {
				compiler_state.debugger_symbols.build(output_file_name);
			}
		}
};

#endif