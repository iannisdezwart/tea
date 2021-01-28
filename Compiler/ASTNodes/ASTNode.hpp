#ifndef TEA_AST_NODE_HEADER
#define TEA_AST_NODE_HEADER

#include <bits/stdc++.h>

using namespace std;

enum ASTNodeType {
	TYPE_IDENTIFIER_PAIR,
	FUNCTION_DECLARATION,
	RETURN_STATEMENT,
	CODE_BLOCK,
	LITERAL_STRING_EXPRESSION,
	LITERAL_CHAR_EXPRESSION,
	LITERAL_NUMBER_EXPRESSION,
	IDENTIFIER_EXPRESSION
};

class ASTNode {
	public:
		ASTNodeType type;

		virtual string to_str() = 0;
		virtual void dfs(function<void(ASTNode *, size_t)>, size_t depth = 0) = 0;

		void print(const char *prefix)
		{
			printf("%s: %s\n", prefix, to_str().c_str());
		}
};

#endif