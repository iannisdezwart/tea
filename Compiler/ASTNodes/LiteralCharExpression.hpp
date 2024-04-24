#ifndef TEA_AST_NODE_LITERAL_CHAR_EXPRESSION_HEADER
#define TEA_AST_NODE_LITERAL_CHAR_EXPRESSION_HEADER

#include "ASTNode.hpp"
#include "ReadValue.hpp"
#include "../tokeniser.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../util.hpp"

struct LiteralCharExpression : public ReadValue
{
	Token literal_char_token;
	uint8_t value;

	LiteralCharExpression(Token literal_char_token)
		: literal_char_token(literal_char_token),
		  value(literal_char_token.value[0]),
		  ReadValue(literal_char_token, LITERAL_CHAR_EXPRESSION) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
	{
		callback(this, depth);
	}

	std::string
	to_str()
	{
		std::string s = "LiteralCharExpression { value = \"";
		s += std::to_string(value);
		s += "\" } @ " + to_hex((size_t) this);
		return s;
	}

	Type
	get_type(CompilerState &compiler_state)
	{
		return Type(Type::UNSIGNED_INTEGER, 1);
	}

	void
	get_value(Assembler &assembler, CompilerState &compiler_state, uint8_t result_reg)
	{
		assembler.move_8_into_reg(value, result_reg);
	}
};

#endif