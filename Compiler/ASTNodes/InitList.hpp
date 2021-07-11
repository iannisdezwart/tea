#ifndef TEA_AST_NODE_INIT_LIST_HEADER
#define TEA_AST_NODE_INIT_LIST_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "ReadValue.hpp"
#include "../tokeniser.hpp"
#include "../compiler-state.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../util.hpp"

using namespace std;

class InitList : public ReadValue {
	public:
		vector<ReadValue *> items;

		InitList(const Token& start_token, vector<ReadValue *>&& items)
			: ReadValue(start_token, INIT_LIST), items(std::move(items)) {}

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

		void get_value(Assembler& assembler, CompilerState& compiler_state)
		{
			// Todo: create
		}
};

#endif