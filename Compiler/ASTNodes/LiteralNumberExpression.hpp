#ifndef TEA_AST_NODE_LITERAL_NUMBER_EXPRESSION_HEADER
#define TEA_AST_NODE_LITERAL_NUMBER_EXPRESSION_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "ReadValue.hpp"
#include "../tokeniser.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../util.hpp"

class LiteralNumberExpression : public ReadValue {
	public:
		Token literal_number_token;
		std::string value;

		LiteralNumberExpression(Token literal_number_token, std::string value)
			: literal_number_token(literal_number_token), value(value),
				ReadValue(literal_number_token, LITERAL_NUMBER_EXPRESSION) {}

		void dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			callback(this, depth);
		}

		std::string to_str()
		{
			std::string s = "LiteralNumberExpression { value = \"" + value + "\" } @ "
				+ to_hex((size_t) this);
			return s;
		}

		uint64_t to_num()
		{
			if (value[0] == '0' && value[1] == 'x')
				return stoull(value.substr(2), NULL, 16);

			if (value[0] == '0' && value[1] == 'b')
				return stoull(value.substr(2), NULL, 2);

			return stoull(value);
		}

		Type get_type(CompilerState& compiler_state)
		{
			Type type(Type::UNSIGNED_INTEGER, 8);

			type.is_literal = true;
			type.literal_value = &value;

			return type;
		}

		void get_value(Assembler& assembler, CompilerState& compiler_state, uint8_t result_reg)
		{
			assembler.move_64_into_reg(to_num(), result_reg);
		}
};

#endif