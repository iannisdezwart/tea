#ifndef TEA_AST_NODE_FUNCTION_DECLARATION_HEADER
#define TEA_AST_NODE_FUNCTION_DECLARATION_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "../tokeniser.hpp"
#include "../util.hpp"
#include "TypeIdentifierPair.hpp"
#include "CodeBlock.hpp"

using namespace std;

class FunctionDeclaration : public ASTNode {
	public:
		TypeIdentifierPair *type_and_id_pair;
		vector<TypeIdentifierPair *> params;
		CodeBlock *body;

		FunctionDeclaration(
			TypeIdentifierPair *type_and_id_pair,
			vector<TypeIdentifierPair *> params,
			CodeBlock *body
		) {
			this->type_and_id_pair = type_and_id_pair;
			this->params = params;
			this->body = body;
			type = FUNCTION_DECLARATION;

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

			for (TypeIdentifierPair *param : params) {
				if (param != NULL)
					param->dfs(callback, depth + 1);
			}

			if (body != NULL)
				body->dfs(callback, depth + 1);

			callback(this, depth);
		}

		string to_str()
		{
			string s = "FunctionDeclaration {} @ " + to_hex((size_t) this);
			return s;
		}
};

#endif