#ifndef TEA_COMPILER_HEADER
#define TEA_COMPILER_HEADER

#if defined(__linux__) && defined(CALLGRIND)
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

#ifdef MEASURE_MEM
	#define MEASURE_MEM_CHECKPOINT(m)            \
		do                                   \
		{                                    \
			std::cout << m << std::endl; \
			std::cin.get();              \
		} while (0)
#else
	#define MEASURE_MEM_CHECKPOINT(m)
#endif

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

	// Whether to print debug information.
	bool debug;

	/**
	 * @brief Constructor.
	 * Opens the input file and initialises the compiler state.
	 * @param input_file_name The name of the source file to be compiled.
	 * @param output_file_name The name of the output file.
	 */
	Compiler(char *input_file_name, char *output_file_name, bool debug)
		: output_file_name(output_file_name), debug(debug)
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

	/**
	 * @brief Pretty-prints the AST to stdout.
	 * The AST is printed in a depth-first post-order traversal.
	 * TODO: Also support pre-order traversal.
	 */
	void
	print_ast(const AST &ast, const std::vector<uint> &statement_nodes)
	{
		p_trace(stdout, "\\\\\\ AST \\\\\\\n\n");

		for (uint statement_node : statement_nodes)
		{
			auto cb = [&ast](uint node, size_t depth)
			{
				for (size_t i = 0; i < depth; i++)
				{
					putc('\t', stdout);
				}

				ast_print(ast, node, "\u279a");
			};

			ast_dfs(ast, statement_node, cb, 0);
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

		MEASURE_MEM_CHECKPOINT("Before parsing");
		Parser parser(tokens);
		auto parse_res          = parser.parse();
		AST &ast                = std::get<0>(parse_res);
		const auto &names_by_id = std::get<1>(parse_res);
		const auto &ids_by_name = std::get<2>(parse_res);
		MEASURE_MEM_CHECKPOINT("After parsing");

		if (debug)
		{
			print_ast(ast, ast.class_declarations);
			print_ast(ast, ast.global_declarations);
			print_ast(ast, ast.function_declarations);
		}

		// Type checking.

		TypeCheckState type_check_state(debug, ast, names_by_id);

		CALLGRIND_START_INSTRUMENTATION;
		CALLGRIND_TOGGLE_COLLECT;

		auto t1 = std::chrono::high_resolution_clock::now();
		MEASURE_MEM_CHECKPOINT("Before type checking");

		for (uint statement_node : ast.class_declarations)
			class_declaration_predefine(ast, statement_node, type_check_state);

		for (uint statement_node : ast.class_declarations)
			ast_type_check(ast, statement_node, type_check_state);
		for (uint statement_node : ast.global_declarations)
			ast_type_check(ast, statement_node, type_check_state);
		for (uint statement_node : ast.function_declarations)
			ast_type_check(ast, statement_node, type_check_state);

		for (uint statement_node : ast.function_declarations)
			function_declaration_type_check_body(ast, statement_node, type_check_state);

		MEASURE_MEM_CHECKPOINT("After type checking");
		auto t2 = std::chrono::high_resolution_clock::now();
		MEASURE_MEM_CHECKPOINT("Before code generation");

		// Start assembling.
		// Allocate space for globals & update stack and frame pointer.

		Assembler assembler(debug, names_by_id);

		assembler.allocate_stack(type_check_state.globals_size);
		uint8_t init_alloc_reg = assembler.get_register();
		assembler.move_lit(type_check_state.globals_size, init_alloc_reg);
		assembler.add_int_64(init_alloc_reg, R_STACK_PTR);
		assembler.add_int_64(init_alloc_reg, R_FRAME_PTR);
		assembler.free_register(init_alloc_reg);

		// Compile intitialisation values for global variables.

		for (uint global_decl_node : ast.global_declarations)
		{
			ast_code_gen(ast, global_decl_node, assembler);
		}

		// Push parameter count and call main.

		auto main_it = ids_by_name.find("main");
		if (main_it == ids_by_name.end())
		{
			err("No main function found");
		}

		uint8_t main_param_cnt_reg = assembler.get_register();
		assembler.move_lit(0, main_param_cnt_reg);
		assembler.push_reg_64(main_param_cnt_reg);
		assembler.free_register(main_param_cnt_reg);
		assembler.call(main_it->second);
		uint exit_label = assembler.generate_label();
		assembler.jump(exit_label);

		// Compile function declarations.

		for (uint function_declaration_node : ast.function_declarations)
		{
			function_declaration_code_gen(ast, function_declaration_node, assembler);
		}

		// Add an exit label.
		// Jumped to after the main function finishes.

		assembler.add_label(exit_label);

		MEASURE_MEM_CHECKPOINT("After code generation");
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