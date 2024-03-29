#ifndef TEA_AST_NODE_LITERAL_STRING_EXPRESSION_HEADER
#define TEA_AST_NODE_LITERAL_STRING_EXPRESSION_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "ReadValue.hpp"
#include "../tokeniser.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../util.hpp"

struct LiteralStringExpression : public ReadValue
{
	Token literal_string_token;
	std::string value;

	LiteralStringExpression(Token literal_string_token, std::string value)
		: literal_string_token(literal_string_token), value(value),
		  ReadValue(literal_string_token, LITERAL_STRING_EXPRESSION) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
	{
		callback(this, depth);
	}

	std::string
	to_str()
	{
		std::string s = "LiteralStringExpression { value = \"" + value + "\" } @ "
			+ to_hex((size_t) this);
		return s;
	}

	Type
	get_type(CompilerState &compiler_state)
	{
		Type type(Type::UNSIGNED_INTEGER, 1, { 0 });
		return type;
	}

	void
	get_value(Assembler &assembler, CompilerState &compiler_state, uint8_t result_reg)
	{
		StaticData static_data = assembler.add_static_data(
			(uint8_t *) value.data(), value.size());

		assembler.move_64_into_reg(static_data.offset, result_reg);
	}
};

#endif