#ifndef TEA_AST_NODE_TYPE_IDENTIFIER_HEADER
#define TEA_AST_NODE_TYPE_IDENTIFIER_HEADER

#include <bits/stdc++.h>

#include "../tokeniser.hpp"
#include "ASTNode.hpp"

using namespace std;

class TypeIdentifierPair : public ASTNode {
	public:
		Token type_token;
		Token identifier_token;

		TypeIdentifierPair(Token type_token, Token identifier_token) {
			this->type_token = type_token;
			this->identifier_token = identifier_token;
			type = TYPE_IDENTIFIER_PAIR;
		}

		string get_identifier_name()
		{
			return identifier_token.value;
		}
};

#endif