#ifndef TEA_AST_NODE_RETURN_STATEMENT_HEADER
#define TEA_AST_NODE_RETURN_STATEMENT_HEADER

#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/ASTNodes/ReadValue.hpp"
#include "Compiler/tokeniser.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/util.hpp"

struct ReturnStatement final : public ASTNode
{
	std::unique_ptr<ReadValue> expression;

	ReturnStatement(Token return_token, std::unique_ptr<ReadValue> expression)
		: ASTNode(std::move(return_token), RETURN_STATEMENT),
		  expression(std::move(expression)) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
		override
	{
		if (expression)
			expression->dfs(callback, depth + 1);

		callback(this, depth);
	}

	std::string
	to_str()
		override
	{
		std::string s = "ReturnStatement {} @ " + to_hex((size_t) this);
		return s;
	}

	void
	type_check(TypeCheckState &type_check_state)
		override
	{
		if (!expression)
			return;

		expression->type_check(type_check_state);
		type = expression->type;
	}

	void
	code_gen(Assembler &assembler)
		const override
	{
		uint8_t res_reg = assembler.get_register();

		// Store value in result register.

		if (expression)
			expression->get_value(assembler, res_reg);

		// Return value.

		assembler.move(res_reg, R_RET);
		assembler.free_register(res_reg);
		assembler.return_();
	}
};

constexpr int RETURN_STATEMENT_SIZE = sizeof(ReturnStatement);

#endif