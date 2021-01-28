#ifndef TEA_AST_NODE_FUNCTION_DECLARATION_HEADER
#define TEA_AST_NODE_FUNCTION_DECLARATION_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "../tokeniser.hpp"
#include "../byte_code.hpp"
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

		void compile(
			unordered_map<string, vector<char>> constants,
			unordered_map<string, size_t> globals,
			unordered_map<string, vector<char>> functions
		) {
			if (functions.count(type_and_id_pair->get_identifier_name()))
				err_at_token(type_and_id_pair->identifier_token,
					"Duplicate identifier name",
					"Identifier %s is already declared",
					type_and_id_pair->get_identifier_name());

			vector<char> instructions;

			
		}
};

#endif