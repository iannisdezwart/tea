#ifndef TEA_AST_NODE_TYPE_IDENTIFIER_HEADER
#define TEA_AST_NODE_TYPE_IDENTIFIER_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "../tokeniser.hpp"
#include "../util.hpp"

using namespace std;

class TypeIdentifierPair : public ASTNode {
	public:
		Token type_token;
		Token identifier_token;

		TypeIdentifierPair(Token type_token, Token identifier_token) {
			this->type_token = type_token;
			this->identifier_token = identifier_token;
			type = TYPE_IDENTIFIER_PAIR;

			#ifdef PARSER_VERBOSE
			print("Created");
			#endif
		}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth) {
			#ifdef PARSER_VERBOSE
			print("dfs");
			#endif

			callback(this, depth);
		}

		string get_identifier_name()
		{
			return identifier_token.value;
		}

		string to_str()
		{
			string s = "TypeIdentifierPair { type = \"" + type_token.value + "\", "
				"identifier = \"" + identifier_token.value + "\" } @ "
				+ to_hex((size_t) this);
			return s;
		}
};

#endif