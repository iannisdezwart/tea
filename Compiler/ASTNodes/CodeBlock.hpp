#ifndef TEA_AST_NODE_CODE_BLOCK_HEADER
#define TEA_AST_NODE_CODE_BLOCK_HEADER

#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/tokeniser.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/code-gen/Assembler.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"

struct CodeBlock final : public ASTNode
{
	std::vector<std::unique_ptr<ASTNode>> statements;

	CodeBlock(Token start_token)
		: ASTNode(std::move(start_token), CODE_BLOCK) {}

	~CodeBlock() {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
		override
	{
		for (const std::unique_ptr<ASTNode> &statement : statements)
		{
			statement->dfs(callback, depth + 1);
		}

		callback(this, depth);
	}

	void
	add_statement(std::unique_ptr<ASTNode> statement)
	{
		statements.push_back(std::move(statement));
	}

	std::string
	to_str()
		override
	{
		std::string s = "CodeBlock {} @ " + to_hex((size_t) this);
		return s;
	}

	void
	type_check(TypeCheckState &type_check_state)
		override
	{
		for (const std::unique_ptr<ASTNode> &statement : statements)
		{
			statement->type_check(type_check_state);
		}
	}

	void
	code_gen(Assembler &assembler)
		const override
	{
		for (const std::unique_ptr<ASTNode> &statement : statements)
		{
			statement->code_gen(assembler);
		}
	}
};

#endif