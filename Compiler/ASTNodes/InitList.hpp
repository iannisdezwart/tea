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

		InitList(Token start_token, vector<ASTNode *>&& items)
			: ASTNode(start_token), items(std::move(items))
		{
			type = INIT_LIST;

			#ifdef PARSER_VERBOSE
			print("Created");
			#endif
		}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth) {
			#ifdef PARSER_VERBOSE
			print("dfs");
			#endif

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

		bool is_compatible_as_class_initialiser(const Class& cl)
		{
			// ...
		}

		void compile(Assembler& assembler, CompilerState& compiler_state)
		{
			// ...
		}
};

#endif