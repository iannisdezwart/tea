#ifndef TEA_AST_NODE_FOR_STATEMENT_HEADER
#define TEA_AST_NODE_FOR_STATEMENT_HEADER

#include "Compiler/ASTNodes/ASTFunctions-fwd.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/CodeBlock.hpp"
#include "Compiler/ASTNodes/AST.hpp"

void
for_statement_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth)
{
	ast_dfs(ast, ast.data[node].for_statement.init_node, callback, depth + 1);

	uint idx = ast.data[node].for_statement.rest_ed_idx;

	uint test_node = ast.extra_data[idx];
	ast_dfs(ast, test_node, callback, depth + 1);

	uint update_node = ast.extra_data[idx + 1];
	ast_dfs(ast, update_node, callback, depth + 1);

	uint body_node = ast.extra_data[idx + 2];
	ast_dfs(ast, body_node, callback, depth + 1);

	callback(node, depth);
}

std::string
for_statement_to_str(const AST &ast, uint node)
{
	return std::string("ForStatement {} @ ") + std::to_string(node);
}

void
for_statement_type_check(AST &ast, uint node, TypeCheckState &type_check_state)
{
	ast_type_check(ast, ast.data[node].for_statement.init_node, type_check_state);

	uint idx = ast.data[node].for_statement.rest_ed_idx;

	uint test_node = ast.extra_data[idx];
	ast_type_check(ast, test_node, type_check_state);

	uint update_node = ast.extra_data[idx + 1];
	ast_type_check(ast, update_node, type_check_state);

	uint body_node = ast.extra_data[idx + 2];
	ast_type_check(ast, body_node, type_check_state);
}

void
for_statement_code_gen(AST &ast, uint node, Assembler &assembler)
{
	uint8_t test_reg;

	// Create labels

	auto [start_label, end_label] = assembler.push_loop_scope();

	// Compile code for the init statement

	uint init_node = ast.data[node].for_statement.init_node;
	ast_code_gen(ast, init_node, assembler);

	// Create the loop label

	assembler.add_label(start_label);

	// Perform the check and move the result into the test register

	test_reg = assembler.get_register();

	uint idx       = ast.data[node].for_statement.rest_ed_idx;
	uint test_node = ast.extra_data[idx];

	ast_get_value(ast, test_node, assembler, test_reg);

	// Break the loop if false

	uint8_t cmp_reg = assembler.get_register();
	assembler.move_lit(0, cmp_reg);
	assembler.cmp_int_8(test_reg, cmp_reg);
	assembler.free_register(cmp_reg);
	assembler.jump_if_eq(end_label);

	assembler.free_register(test_reg);

	// Compile code for the body block

	uint body_node = ast.extra_data[idx + 2];
	ast_code_gen(ast, body_node, assembler);

	// Compile code for the update expression

	uint update_node = ast.extra_data[idx + 1];
	ast_code_gen(ast, update_node, assembler);

	// Jump to the start of the loop again

	assembler.jump(start_label);

	// Create the end label

	assembler.add_label(end_label);

	assembler.pop_loop_scope();
}

#endif