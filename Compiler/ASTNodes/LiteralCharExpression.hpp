#ifndef TEA_AST_NODE_LITERAL_CHAR_EXPRESSION_HEADER
#define TEA_AST_NODE_LITERAL_CHAR_EXPRESSION_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../util.hpp"

using namespace std;

class LiteralCharExpression : public ASTNode {
	public:
		Token literal_char_token;
		uint8_t value;

		LiteralCharExpression(Token literal_char_token, uint8_t value)
			: literal_char_token(literal_char_token),
				ASTNode(literal_char_token), value(value)
		{
			type = LITERAL_CHAR_EXPRESSION;

			#ifdef PARSER_VERBOSE
			print("Created");
			#endif
		}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			#ifdef PARSER_VERBOSE
			print("dfs");
			#endif

			callback(this, depth);
		}

		string to_str()
		{
			string s = "LiteralCharExpression { value = \"" + to_string(value) +
				"\" } @ " + to_hex((size_t) this);
			return s;
		}

		Type get_type(CompilerState& compiler_state)
		{
			return Type(Type::UNSIGNED_INTEGER, 1);
		}

		void compile(Assembler& assembler, CompilerState& compiler_state) {
			assembler.move_8_into_reg(value, R_ACCUMULATOR_0_ID);
		}
};

#endif