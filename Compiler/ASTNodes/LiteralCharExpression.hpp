#ifndef TEA_AST_NODE_LITERAL_CHAR_EXPRESSION_HEADER
#define TEA_AST_NODE_LITERAL_CHAR_EXPRESSION_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../util.hpp"

using namespace std;

class LiteralCharExpression : public ASTNode {
	public:
		uint8_t value;

		LiteralCharExpression(uint8_t value)
		{
			this->value = value;
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

		void compile(Assembler& assembler, CompilerState& compiler_state) {
			assembler.move_8_into_reg(value, R_ACCUMULATOR_0_ID);
		}
};

#endif