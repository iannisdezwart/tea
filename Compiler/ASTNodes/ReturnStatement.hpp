#ifndef TEA_AST_NODE_RETURN_STATEMENT_HEADER
#define TEA_AST_NODE_RETURN_STATEMENT_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"

using namespace std;

class ReturnStatement : public ASTNode {
	public:
		int a;

		ReturnStatement(int a) {
			this->a = a;
			type = RETURN_STATEMENT;
		}
};

#endif