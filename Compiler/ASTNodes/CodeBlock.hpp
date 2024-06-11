#ifndef TEA_AST_NODE_CODE_BLOCK_HEADER
#define TEA_AST_NODE_CODE_BLOCK_HEADER

#include "Compiler/ASTNodes/ASTFunctions-fwd.hpp"
#include "Compiler/util.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/code-gen/Assembler.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"
#include "Compiler/ASTNodes/AST.hpp"

void
code_block_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth)
{
	uint statement_idx = ast.data[node].code_block.statements_ed_idx;
	uint len           = ast.data[node].code_block.statements_len;
	for (uint i = statement_idx; i < statement_idx + len; i++)
	{
		uint statement_node = ast.extra_data[i];
		ast_dfs(ast, statement_node, callback, depth + 1);
	}

	callback(node, depth);
}

std::string
code_block_to_str(const AST &ast, uint node)
{
	return std::string("CodeBlock {} @ ") + std::to_string(node);
}

void
code_block_type_check(AST &ast, uint node, TypeCheckState &type_check_state)
{
	type_check_state.begin_local_scope();

	uint statement_idx = ast.data[node].code_block.statements_ed_idx;
	uint len           = ast.data[node].code_block.statements_len;
	for (uint i = statement_idx; i < statement_idx + len; i++)
	{
		uint statement_node = ast.extra_data[i];
		ast_type_check(ast, statement_node, type_check_state);
	}

	type_check_state.end_local_scope();
}

void
code_block_code_gen(AST &ast, uint node, Assembler &assembler)
{
	uint statement_idx = ast.data[node].code_block.statements_ed_idx;
	uint len           = ast.data[node].code_block.statements_len;
	for (uint i = statement_idx; i < statement_idx + len; i++)
	{
		uint statement_node = ast.extra_data[i];
		ast_code_gen(ast, statement_node, assembler);
	}
}

#endif