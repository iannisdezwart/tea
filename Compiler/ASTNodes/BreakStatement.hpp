#ifndef TEA_AST_NODE_BREAK_STATEMENT_HEADER
#define TEA_AST_NODE_BREAK_STATEMENT_HEADER

#include "Compiler/ASTNodes/ASTNode.hpp"

struct BreakStatement final : public ASTNode
{
	BreakStatement(CompactToken accountable_token)
		: ASTNode(std::move(accountable_token), BREAK_STATEMENT) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
		override
	{
		callback(this, depth);
	}

	std::string
	to_str()
		override
	{
		std::string s = "BreakStatement {} @ " + to_hex((size_t) this);
		return s;
	}

	void
	type_check(TypeCheckState &type_check_state)
		override
	{
	}

	void
	code_gen(Assembler &assembler)
		const override
	{
		// Get the loop label.

		auto [_, end_label] = assembler.loop_labels.top();

		// Jump to the loop label.

		assembler.jump(end_label);
	}
};

#endif