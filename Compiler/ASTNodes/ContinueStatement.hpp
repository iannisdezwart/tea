#ifndef TEA_AST_NODE_CONTINUE_STATEMENT_HEADER
#define TEA_AST_NODE_CONTINUE_STATEMENT_HEADER

#include "Compiler/ASTNodes/ASTFunctions-fwd.hpp"
#include "Compiler/ASTNodes/AST.hpp"

void
continue_statement_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth)
{
	callback(node, depth);
}

std::string
continue_statement_to_str(const AST &ast, uint node)
{
	return std::string("ContinueStatement {} @ ") + std::to_string(node);
}

void
continue_statement_code_gen(Assembler &assembler)
{
	// Get the loop label.

	auto [start_label, _] = assembler.loop_labels.top();

	// Jump to the loop label.

	assembler.jump(start_label);
}

#endif