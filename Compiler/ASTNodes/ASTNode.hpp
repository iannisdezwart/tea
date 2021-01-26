#ifndef TEA_AST_NODE_HEADER
#define TEA_AST_NODE_HEADER

#include <bits/stdc++.h>

using namespace std;

enum ASTNodeType {
	TYPE_IDENTIFIER_PAIR,
	FUNCTION_DECLARATION,
	RETURN_STATEMENT,
	CODE_BLOCK
};

class ASTNode {
	public:
		ASTNodeType type;
};

#endif