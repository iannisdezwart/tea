#ifndef TEA_COMPILER_HEADER
#define TEA_COMPILER_HEADER

#ifdef __linux__
#include <valgrind/callgrind.h>
#else
#define CALLGRIND_START_INSTRUMENTATION
#define CALLGRIND_TOGGLE_COLLECT
#define CALLGRIND_STOP_INSTRUMENTATION
#endif
#include <chrono>
#include "Compiler/ASTNodes/ClassDeclaration.hpp"
#include "Compiler/util.hpp"
#include "Compiler/parser.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"
#include "debugger-symbols.hpp"
#include "Compiler/code-gen/Assembler.hpp"
#include "ASTNodes/VariableDeclaration.hpp"

/**
 * @brief Class responsible for compiling a source file into a byte code file.
 * Takes a source file name and compiles this file it into a given output file.
 */
struct Compiler
{
	// A file pointer to the source file.
	FILE *input_file;

	// The output file name.
	// Will contain the byte code after compilation.
	char *output_file_name;

	// The assembler object.
	Assembler assembler;

	// The compiler state object.
	TypeCheckState type_check_state;

	/**
	 * @brief Constructor.
	 * Opens the input file and initialises the compiler state.
	 * @param input_file_name The name of the source file to be compiled.
	 * @param output_file_name The name of the output file.
	 */
	Compiler(char *input_file_name, char *output_file_name, bool debug)
		: output_file_name(output_file_name),
		  assembler(debug),
		  type_check_state(debug)
	{
		input_file = fopen(input_file_name, "r");

		if (input_file == nullptr)
		{
			err("Input file %s does not exist", input_file_name);
		}
	}

	/**
	 * @brief Destructor.
	 * Closes the input file.
	 */
	~Compiler()
	{
		fclose(input_file);
	}

	void
	collect_decl(std::vector<VariableDeclaration *> &global_var_decls,
		std::vector<FunctionDeclaration *> &fn_decls,
		std::vector<ClassDeclaration *> &class_decls,
		const std::unique_ptr<ASTNode> &statement)
	{
		switch (statement->node_type)
		{
		case VARIABLE_DECLARATION:
			global_var_decls.push_back((VariableDeclaration *) statement.get());
			break;

		case FUNCTION_DECLARATION:
			fn_decls.push_back((FunctionDeclaration *) statement.get());
			break;

		case CLASS_DECLARATION:
		{
			class_decls.push_back((ClassDeclaration *) statement.get());
			break;
		}

		default:
			err_at_token(statement->accountable_token, "Unexpected statement",
				"Unexpected statement of type %s",
				ast_node_type_to_str(statement->node_type));
			break;
		}
	}

	/**
	 * @brief Pretty-prints the AST to stdout.
	 * The AST is printed in a depth-first post-order traversal.
	 * TODO: Also support pre-order traversal.
	 */
	void
	print_ast(const std::vector<std::unique_ptr<ASTNode>> &statements)
	{
		p_trace(stdout, "\\\\\\ AST \\\\\\\n\n");

		for (const std::unique_ptr<ASTNode> &statement : statements)
		{
			auto cb = [](ASTNode *node, size_t depth)
			{
				for (size_t i = 0; i < depth; i++)
				{
					putc('\t', stdout);
				}

				node->print("\u279a");
			};

			statement->dfs(cb);
		}

		p_trace(stdout, "\n/// AST ///\n");
	}

	/**
	 * @brief Compiles the source file into a byte code file.
	 * Tokenises the source file,
	 * parses the tokens and compiles them into byte code.
	 */
	void
	compile()
	{
		// Tokenise the source file.

		Tokeniser tokeniser(input_file);
		std::vector<Token> tokens = tokeniser.tokenise();
		tokeniser.print_tokens();

		// Parse the tokens.

		Parser parser(tokens);
		std::vector<std::unique_ptr<ASTNode>> statements = parser.parse();
		print_ast(statements);

		// Type checking.

		CALLGRIND_START_INSTRUMENTATION;
		CALLGRIND_TOGGLE_COLLECT;

		auto t1 = std::chrono::high_resolution_clock::now();

		for (const std::unique_ptr<ASTNode> &statement : statements)
			statement->pre_type_check(type_check_state);
		for (const std::unique_ptr<ASTNode> &statement : statements)
			statement->type_check(type_check_state);
		for (const std::unique_ptr<ASTNode> &statement : statements)
			statement->post_type_check(type_check_state);

		// Collect declarations.

		std::vector<VariableDeclaration *> global_var_decls;
		std::vector<FunctionDeclaration *> fn_decls;
		std::vector<ClassDeclaration *> class_decls;

		for (const std::unique_ptr<ASTNode> &statement : statements)
		{
			collect_decl(global_var_decls, fn_decls, class_decls, statement);
		}

		auto t2 = std::chrono::high_resolution_clock::now();

		// Start assembling.
		// Allocate space for globals & update stack and frame pointer.

		assembler.allocate_stack(type_check_state.globals_size);
		uint8_t init_alloc_reg = assembler.get_register();
		assembler.move_lit(type_check_state.globals_size, init_alloc_reg);
		assembler.add_int_64(init_alloc_reg, R_STACK_PTR);
		assembler.add_int_64(init_alloc_reg, R_FRAME_PTR);
		assembler.free_register(init_alloc_reg);

		// Compile intitialisation values for global variables.

		for (VariableDeclaration *decl : global_var_decls)
		{
			decl->code_gen(assembler);
		}

		// Push parameter count and call main.

		uint8_t main_param_cnt_reg = assembler.get_register();
		assembler.move_lit(0, main_param_cnt_reg);
		assembler.push_reg_64(main_param_cnt_reg);
		assembler.free_register(main_param_cnt_reg);
		assembler.call("main");
		assembler.jump("exit");

		// Compile function declarations.

		for (FunctionDeclaration *decl : fn_decls)
		{
			decl->code_gen(assembler);
		}

		// Compile class method declarations.

		for (ClassDeclaration *decl : class_decls)
		{
			decl->code_gen(assembler);
		}

		// Add an exit label.
		// Jumped to after the main function finishes.

		assembler.add_label("exit");

		auto t3 = std::chrono::high_resolution_clock::now();

		CALLGRIND_TOGGLE_COLLECT;
		CALLGRIND_STOP_INSTRUMENTATION;

		// Print the time taken for type checking and code generation.

		auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
		auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();
		p_warn(stdout, "Type checking took %lld micros\n", duration1);
		p_warn(stdout, "Code generation took %lld micros\n", duration2);

		// Write the byte code to the output file.

		Buffer executable = assembler.assemble();
		executable.write_to_file(output_file_name);

		// Create debugger symbols

		if (type_check_state.debug)
		{
			type_check_state.debugger_symbols.build(output_file_name);
		}
	}
};

#endif