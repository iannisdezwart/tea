#ifndef TEA_AST_NODE_INIT_LIST_HEADER
#define TEA_AST_NODE_INIT_LIST_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "ReadValue.hpp"
#include "../tokeniser.hpp"
#include "../compiler-state.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../util.hpp"

struct InitList : public ReadValue {
		std::vector<ReadValue *> items;

		InitList(const Token& start_token, std::vector<ReadValue *>&& items)
			: ReadValue(start_token, INIT_LIST), items(std::move(items)) {}

		void dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth) {
			for (ASTNode *item : items) {
				item->dfs(callback, depth + 1);
			}

			callback(this, depth);
		}

		std::string to_str()
		{
			std::string s = "InitList {} @ " + to_hex((size_t) this);
			return s;
		}

		Type get_type(CompilerState& compiler_state)
		{
			std::vector<Type> fields;
			size_t total_size;

			for (ReadValue *item : items) {
				Type item_type = item->get_type(compiler_state);
				total_size += item_type.byte_size();
				fields.push_back(std::move(item_type));
			}

			Type type(Type::INIT_LIST, total_size);
			type.fields = std::move(fields);
			return type;
		}

		void get_value(Assembler& assembler, CompilerState& compiler_state, uint8_t result_reg)
		{
			// Todo: create
		}
};

#endif