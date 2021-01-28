#ifndef TEA_AST_NODE_RETURN_STATEMENT_HEADER
#define TEA_AST_NODE_RETURN_STATEMENT_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "../util.hpp"

using namespace std;

class ReturnStatement : public ASTNode {
	public:
		ASTNode *expression;

		ReturnStatement(ASTNode *expression) {
			this->expression = expression;
			type = RETURN_STATEMENT;

			#ifdef PARSER_VERBOSE
			print("Created");
			#endif
		}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			#ifdef PARSER_VERBOSE
			print("dfs");
			#endif

			if (expression != NULL)
				expression->dfs(callback, depth + 1);

			callback(this, depth);
		}

		string to_str()
		{
			string s = "ReturnStatement {} @ " + to_hex((size_t) this);
			return s;
		}
};

#endif