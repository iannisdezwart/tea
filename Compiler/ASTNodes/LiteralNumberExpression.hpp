#ifndef TEA_AST_NODE_LITERAL_NUMBER_EXPRESSION_HEADER
#define TEA_AST_NODE_LITERAL_NUMBER_EXPRESSION_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "../tokeniser.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../util.hpp"

using namespace std;

class LiteralNumberExpression : public ASTNode {
	public:
		Token literal_number_token;
		string value;

		LiteralNumberExpression(Token literal_number_token, string value)
			: literal_number_token(literal_number_token), value(value),
				ASTNode(literal_number_token, LITERAL_NUMBER_EXPRESSION) {}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			callback(this, depth);
		}

		string to_str()
		{
			string s = "LiteralNumberExpression { value = \"" + value + "\" } @ "
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
			return Type(Type::UNSIGNED_INTEGER, 8);
		}

		void compile(Assembler& assembler, CompilerState& compiler_state)
		{
			assembler.move_64_into_reg(to_num(), R_ACCUMULATOR_0_ID);
		}
};

#endif