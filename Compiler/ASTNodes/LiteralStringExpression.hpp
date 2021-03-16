#ifndef TEA_AST_NODE_LITERAL_STRING_EXPRESSION_HEADER
#define TEA_AST_NODE_LITERAL_STRING_EXPRESSION_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../util.hpp"

using namespace std;

class LiteralStringExpression : public ASTNode {
	public:
		string value;

		LiteralStringExpression(string value)
		{
			this->value = value;
			type = LITERAL_STRING_EXPRESSION;

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
			string s = "LiteralStringExpression { value = \"" + value + "\" } @ "
				+ to_hex((size_t) this);
			return s;
		}

		void compile(Assembler& assembler, CompilerState& compiler_state) {
			StaticData static_data = assembler.add_static_data(
				(uint8_t *) value.data(), value.size());

			assembler.move_64_into_reg(PROGRAM_START + static_data.offset,
				R_ACCUMULATOR_0_ID);
		}
};

#endif