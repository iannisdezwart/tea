#ifndef TEA_AST_NODE_LITERAL_STRING_EXPRESSION_HEADER
#define TEA_AST_NODE_LITERAL_STRING_EXPRESSION_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "ReadValue.hpp"
#include "../tokeniser.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../util.hpp"

using namespace std;

class LiteralStringExpression : public ReadValue {
	public:
		Token literal_string_token;
		string value;

		LiteralStringExpression(Token literal_string_token, string value)
			: literal_string_token(literal_string_token), value(value),
				ReadValue(literal_string_token, LITERAL_STRING_EXPRESSION) {}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			callback(this, depth);
		}

		string to_str()
		{
			string s = "LiteralStringExpression { value = \"" + value + "\" } @ "
				+ to_hex((size_t) this);
			return s;
		}

		Type get_type(CompilerState& compiler_state)
		{
			Type type(Type::UNSIGNED_INTEGER, 1, { 0 });
			return type;
		}

		void get_value(Assembler& assembler, CompilerState& compiler_state, uint8_t result_reg)
		{
			StaticData static_data = assembler.add_static_data(
				(uint8_t *) value.data(), value.size());

			assembler.move_64_into_reg(static_data.offset, result_reg);
		}
};

#endif