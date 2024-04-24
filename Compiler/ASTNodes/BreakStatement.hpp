#ifndef TEA_AST_NODE_BREAK_STATEMENT_HEADER
#define TEA_AST_NODE_BREAK_STATEMENT_HEADER

#include "ASTNode.hpp"
#include "../tokeniser.hpp"

struct BreakStatement : public ASTNode
{
	Token break_token;

	BreakStatement(const Token &break_token)
		: break_token(break_token),
		  ASTNode(break_token, BREAK_STATEMENT) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
	{
		callback(this, depth);
	}

	std::string
	to_str()
	{
		std::string s = "BreakStatement {} @ " + to_hex((size_t) this);
		return s;
	}

	Type
	get_type(CompilerState &compiler_state)
	{
		return Type();
	}

	void
	compile(Assembler &assembler, CompilerState &compiler_state)
	{
		// Get the loop label.

		auto [_, end_label] = compiler_state.loop_labels.top();

		// Jump to the loop label.

		assembler.jump(end_label);
	}
};

#endif