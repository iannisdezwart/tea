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
		uint8_t res_reg = assembler.get_register();

		// Store value in result register.

		if (expression != NULL)
			expression->get_value(assembler, compiler_state, res_reg);

		// Return value.

		assembler.move_reg_into_reg(res_reg, R_RET);
		assembler.free_register(res_reg);
		assembler.return_();
	}
};

#endif