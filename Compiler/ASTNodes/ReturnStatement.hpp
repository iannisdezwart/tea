#ifndef TEA_AST_NODE_RETURN_STATEMENT_HEADER
#define TEA_AST_NODE_RETURN_STATEMENT_HEADER

#include "ASTNode.hpp"
#include "ReadValue.hpp"
#include "../tokeniser.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../util.hpp"

struct ReturnStatement : public ASTNode
{
	Token return_token;
	ReadValue *expression;

	ReturnStatement(Token return_token, ReadValue *expression)
		: expression(expression), return_token(return_token),
		  ASTNode(return_token, RETURN_STATEMENT) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
	{
		if (expression != NULL)
			expression->dfs(callback, depth + 1);

		callback(this, depth);
	}

	std::string
	to_str()
	{
		std::string s = "ReturnStatement {} @ " + to_hex((size_t) this);
		return s;
	}

	Type
	get_type(CompilerState &compiler_state)
	{
		if (expression == NULL)
			return Type();
		return expression->get_type(compiler_state);
	}

	void
	compile(Assembler &assembler, CompilerState &compiler_state)
	{
		// Moves the return value into R_RET

		if (expression != NULL)
			expression->get_value(assembler, compiler_state, R_RET);

		assembler.return_();
	}
};

#endif