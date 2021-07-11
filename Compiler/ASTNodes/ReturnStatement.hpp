#ifndef TEA_AST_NODE_RETURN_STATEMENT_HEADER
#define TEA_AST_NODE_RETURN_STATEMENT_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "../tokeniser.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../util.hpp"

using namespace std;

class ReturnStatement : public ASTNode {
	public:
		Token return_token;
		ASTNode *expression;

		ReturnStatement(Token return_token, ASTNode *expression)
			: expression(expression), return_token(return_token),
				ASTNode(return_token, RETURN_STATEMENT) {}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			if (expression != NULL)
				expression->dfs(callback, depth + 1);

			callback(this, depth);
		}

		string to_str()
		{
			string s = "ReturnStatement {} @ " + to_hex((size_t) this);
			return s;
		}

		Type get_type(CompilerState& compiler_state)
		{
			if (expression == NULL) return Type();
			return expression->get_type(compiler_state);
		}

		void compile(Assembler& assembler, CompilerState& compiler_state)
		{
			// Moves the return value into R_ACCUMULATOR_0

			if (expression != NULL) expression->compile(assembler, compiler_state);

			assembler.return_();
		}
};

#endif