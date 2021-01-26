#ifndef TEA_AST_NODE_CODE_BLOCK_HEADER
#define TEA_AST_NODE_CODE_BLOCK_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"

using namespace std;

class CodeBlock : public ASTNode {
	public:
		vector<ASTNode *> nodes;

		CodeBlock()
		{
			type = CODE_BLOCK;
		}
};

#endif