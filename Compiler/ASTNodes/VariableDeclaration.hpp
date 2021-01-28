#ifndef TEA_AST_NODE_VARIABLE_DECLARATION_HEADER
#define TEA_AST_NODE_VARIABLE_DECLARATION_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "../tokeniser.hpp"
#include "../byte_code.hpp"
#include "../util.hpp"
#include "TypeIdentifierPair.hpp"

using namespace std;

class VariableDeclaration : public ASTNode {
	public:
		TypeIdentifierPair *type_and_id_pair;
		ASTNode *expression;

		VariableDeclaration(
			TypeIdentifierPair *type_and_id_pair,
			ASTNode *expression
		) {
			this->type_and_id_pair = type_and_id_pair;
			this->expression = expression;
			type = VARIABLE_DECLARATION;

			#ifdef PARSER_VERBOSE
			print("Created");
			#endif
		}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			#ifdef PARSER_VERBOSE
			print("dfs");
			#endif

			if (type_and_id_pair != NULL)
				type_and_id_pair->dfs(callback, depth + 1);

			if (expression != NULL)
				expression->dfs(callback, depth + 1);

			callback(this, depth);
		}

		string to_str()
		{
			string s = "VariableDeclaration {} @ " + to_hex((size_t) this);
			return s;
		}
};

#endif