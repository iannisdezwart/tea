#ifndef TEA_AST_NODE_BREAK_STATEMENT_HEADER
#define TEA_AST_NODE_BREAK_STATEMENT_HEADER

#include "Compiler/ASTNodes/ASTFunctions-fwd.hpp"
#include "Compiler/ASTNodes/AST.hpp"

void
break_statement_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth)
{
	callback(node, depth);
}

std::string
break_statement_to_str(const AST &ast, uint node)
{
	return std::string("BreakStatement {} @ ") + std::to_string(node);
}

void
break_statement_code_gen(Assembler &assembler)
{
	// Get the loop label.

	auto [_, end_label] = assembler.loop_labels.top();

	// Jump to the loop label.

	assembler.jump(end_label);
}

#endif