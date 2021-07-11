#ifndef TEA_AST_NODE_INIT_LIST_HEADER
#define TEA_AST_NODE_INIT_LIST_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "../tokeniser.hpp"
#include "../compiler-state.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../util.hpp"

using namespace std;

class InitList : public ASTNode {
	public:
		vector<ASTNode *> items;

		InitList(const Token& start_token, vector<ASTNode *>&& items)
			: ASTNode(start_token, INIT_LIST), items(std::move(items)) {}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth) {
			for (ASTNode *item : items) {
				item->dfs(callback, depth + 1);
			}

			callback(this, depth);
		}

		string to_str()
		{
			string s = "InitList {} @ " + to_hex((size_t) this);
			return s;
		}

		Type get_type(CompilerState& compiler_state)
		{
			return Type();
		}

		void compile(Assembler& assembler, CompilerState& compiler_state)
		{
			// ...
		}
};

#endif