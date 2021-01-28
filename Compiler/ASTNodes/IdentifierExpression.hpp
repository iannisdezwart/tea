#ifndef TEA_AST_NODE_IDENTIFIER_EXPRESSION_HEADER
#define TEA_AST_NODE_IDENTIFIER_EXPRESSION_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "../util.hpp"
#include "../tokeniser.hpp"

using namespace std;

class IdentifierExpression : public ASTNode {
	public:
		Token identifier_token;

		IdentifierExpression(Token identifier_token)
		{
			this->identifier_token = identifier_token;
			type = IDENTIFIER_EXPRESSION;

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
			string s = "IdentifierExpression { identifier = \""
				+ identifier_token.value + "\" } @ " + to_hex((size_t) this);
			return s;
		}
};

#endif