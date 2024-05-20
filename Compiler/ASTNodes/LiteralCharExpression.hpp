#ifndef TEA_AST_NODE_LITERAL_CHAR_EXPRESSION_HEADER
#define TEA_AST_NODE_LITERAL_CHAR_EXPRESSION_HEADER

#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/ASTNodes/ReadValue.hpp"
#include "Compiler/tokeniser.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/util.hpp"

struct LiteralCharExpression final : public ReadValue
{
	uint8_t value;

	LiteralCharExpression(Token literal_char_token)
		: ReadValue(std::move(literal_char_token), LITERAL_CHAR_EXPRESSION),
		  value(accountable_token.value[0]) {}

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
		std::string s = "LiteralCharExpression { value = \"";
		s += std::to_string(value);
		s += "\" } @ " + to_hex((size_t) this);
		return s;
	}

	void
	type_check(TypeCheckState &type_check_state)
		override
	{
		type = Type(Type::UNSIGNED_INTEGER, 1);
	}

	void
	get_value(Assembler &assembler, uint8_t result_reg)
		const override
	{
		assembler.move_lit(value, result_reg);
	}
};

constexpr int LITERAL_CHAR_EXPRESSION_SIZE = sizeof(LiteralCharExpression);

#endif