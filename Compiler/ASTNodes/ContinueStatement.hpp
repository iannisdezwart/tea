#ifndef TEA_AST_NODE_CONTINUE_STATEMENT_HEADER
#define TEA_AST_NODE_CONTINUE_STATEMENT_HEADER

#include "ASTNode.hpp"
#include "../tokeniser.hpp"

struct ContinueStatement : public ASTNode
{
	Token continue_token;

	ContinueStatement(const Token &continue_token)
		: continue_token(continue_token),
		  ASTNode(continue_token, CONTINUE_STATEMENT) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
	{
		callback(this, depth);
	}

	std::string
	to_str()
	{
		std::string s = "ContinueStatement {} @ " + to_hex((size_t) this);
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

		auto [start_label, _] = compiler_state.loop_labels.top();

		// Jump to the loop label.

		assembler.jump(start_label);
	}
};

#endif