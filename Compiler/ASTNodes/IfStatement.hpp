#ifndef TEA_AST_NODE_IF_STATEMENT_HEADER
#define TEA_AST_NODE_IF_STATEMENT_HEADER

#include "Compiler/ASTNodes/ASTFunctions-fwd.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/CodeBlock.hpp"
#include "Compiler/ASTNodes/AST.hpp"

void
if_statement_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth)
{
	ast_dfs(ast, ast.data[node].if_statement.condition_node, callback, depth + 1);
	ast_dfs(ast, ast.data[node].if_statement.then_block_node, callback, depth + 1);

	callback(node, depth);
}

void
if_else_statement_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth)
{
	ast_dfs(ast, ast.data[node].if_else_statement.condition_node, callback, depth + 1);

	uint ed_idx = ast.data[node].if_else_statement.ed_idx;

	uint then_block = ast.extra_data[ed_idx];
	ast_dfs(ast, then_block, callback, depth + 1);

	uint else_block = ast.extra_data[ed_idx + 1];
	ast_dfs(ast, else_block, callback, depth + 1);

	callback(node, depth);
}

std::string
if_statement_to_str(const AST &ast, uint node)
{
	return std::string("IfStatement {} @ ") + std::to_string(node);
}

std::string
if_else_statement_to_str(const AST &ast, uint node)
{
	return std::string("IfElseStatement {} @ ") + std::to_string(node);
}

void
if_statement_type_check(AST &ast, uint node, TypeCheckState &type_check_state)
{
	ast_type_check(ast, ast.data[node].if_statement.condition_node, type_check_state);
	ast_type_check(ast, ast.data[node].if_statement.then_block_node, type_check_state);
}

void
if_else_statement_type_check(AST &ast, uint node, TypeCheckState &type_check_state)
{
	ast_type_check(ast, ast.data[node].if_else_statement.condition_node, type_check_state);

	uint ed_idx = ast.data[node].if_else_statement.ed_idx;

	uint then_block = ast.extra_data[ed_idx];
	ast_type_check(ast, then_block, type_check_state);

	uint else_block = ast.extra_data[ed_idx + 1];
	ast_type_check(ast, else_block, type_check_state);
}

void
if_statement_code_gen(AST &ast, uint node, Assembler &assembler)
{
	uint8_t test_reg;

	// Create labels

	uint end_label = assembler.generate_label();

	// Perform the check and move the result into the test register

	test_reg       = assembler.get_register();
	uint test_node = ast.data[node].if_statement.condition_node;
	ast_get_value(ast, test_node, assembler, test_reg);

	// Jump to the right label

	uint8_t cmp_reg = assembler.get_register();
	assembler.move_lit(0, cmp_reg);
	assembler.cmp_int_8(test_reg, cmp_reg);
	assembler.free_register(cmp_reg);
	assembler.jump_if_eq(end_label);

	assembler.free_register(test_reg);

	// Compile code for then block

	uint then_block = ast.data[node].if_statement.then_block_node;
	ast_code_gen(ast, then_block, assembler);
	assembler.jump(end_label);

	// Create the end label

	assembler.add_label(end_label);
}

void
if_else_statement_code_gen(AST &ast, uint node, Assembler &assembler)
{
	uint8_t test_reg;

	// Create labels

	uint else_label = assembler.generate_label();
	uint end_label  = assembler.generate_label();

	// Perform the check and move the result into the test register

	test_reg       = assembler.get_register();
	uint test_node = ast.data[node].if_else_statement.condition_node;
	ast_get_value(ast, test_node, assembler, test_reg);

	// Jump to the right label

	uint8_t cmp_reg = assembler.get_register();
	assembler.move_lit(0, cmp_reg);
	assembler.cmp_int_8(test_reg, cmp_reg);
	assembler.free_register(cmp_reg);
	assembler.jump_if_eq(else_label);

	assembler.free_register(test_reg);

	// Compile code for then block

	uint ed_idx     = ast.data[node].if_else_statement.ed_idx;
	uint then_block = ast.extra_data[ed_idx];
	ast_code_gen(ast, then_block, assembler);
	assembler.jump(end_label);

	// Compile code for else block

	assembler.add_label(else_label);
	uint else_block = ast.extra_data[ed_idx + 1];
	ast_code_gen(ast, else_block, assembler);

	// Create the end label

	assembler.add_label(end_label);
}

#endif