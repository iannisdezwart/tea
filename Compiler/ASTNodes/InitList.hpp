#ifndef TEA_AST_NODE_INIT_LIST_HEADER
#define TEA_AST_NODE_INIT_LIST_HEADER

#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/ASTNodes/ReadValue.hpp"
#include "Compiler/tokeniser.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/util.hpp"

struct InitList final : public ReadValue
{
	std::vector<std::unique_ptr<ReadValue>> items;

	InitList(Token start_token, std::vector<std::unique_ptr<ReadValue>> &&items)
		: ReadValue(std::move(start_token), INIT_LIST),
		  items(std::move(items)) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
		override
	{
		for (std::unique_ptr<ReadValue> &item : items)
		{
			item->dfs(callback, depth + 1);
		}

		callback(this, depth);
	}

	std::string
	to_str()
		override
	{
		std::string s = "InitList {} @ " + to_hex((size_t) this);
		return s;
	}

	void
	type_check(TypeCheckState &type_check_state)
		override
	{
		std::vector<Type> fields;
		size_t total_size = 0;

		for (std::unique_ptr<ReadValue> &item : items)
		{
			item->type_check(type_check_state);
			total_size += item->type.byte_size();
			fields.push_back(item->type);
		}

		type        = Type(Type::INIT_LIST, total_size);
		type.fields = std::move(fields);
	}

	void
	get_value(Assembler &assembler, uint8_t result_reg)
		const override
	{
		// Todo: create
	}
};

#endif