#ifndef TEA_AST_NODE_FUNCTION_DECLARATION_HEADER
#define TEA_AST_NODE_FUNCTION_DECLARATION_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "../tokeniser.hpp"

using namespace std;

class FunctionDeclaration : public ASTNode {
	public:
		Token identifier;
		string function_name;
		CodeBlock body;

		FunctionDeclaration(string function_name) {
			this->function_name = function_name;
			type = FUNCTION_DECLARATION;
		}
};

#endif