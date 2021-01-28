#ifndef TEA_AST_NODE_CODE_BLOCK_HEADER
#define TEA_AST_NODE_CODE_BLOCK_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "../util.hpp"

using namespace std;

class CodeBlock : public ASTNode {
	public:
		vector<ASTNode *> statements;

		CodeBlock()
		{
			type = CODE_BLOCK;

			#ifdef PARSER_VERBOSE
			print("Created");
			#endif
		}

		~CodeBlock() {}

		void add_statement(ASTNode *statement)
		{
			statements.push_back(statement);
		}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			#ifdef PARSER_VERBOSE
			print("dfs");
			#endif

			for (ASTNode *statement : statements) {
				if (statement != NULL)
					statement->dfs(callback, depth + 1);
			}

			callback(this, depth);
		}

		string to_str()
		{
			string s = "CodeBlock {} @ " + to_hex((size_t) this);
			return s;
		}
};

#endif