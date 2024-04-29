#ifndef TEA_AST_NODE_TYPE_NAME_HEADER
#define TEA_AST_NODE_TYPE_NAME_HEADER

#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/tokeniser.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"
#include "Compiler/util.hpp"

struct TypeName final : public ASTNode
{
	std::vector<size_t> array_sizes;

	TypeName(Token type_token, std::vector<size_t> &&array_sizes)
		: ASTNode(std::move(type_token), TYPE_NAME),
		  array_sizes(std::move(array_sizes)) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
		override
	{
		callback(this, depth);
	}

	size_t
	pointer_depth()
	{
		return array_sizes.size();
	}

	std::string
	type_to_str()
	{
		std::string out;

		out += accountable_token.value;

		for (size_t i = 0; i < array_sizes.size(); i++)
		{
			if (array_sizes[i] == 0)
			{
				out += '*';
			}
			else
			{
				out += '[';
				out += std::to_string(array_sizes[i]);
				out += ']';
			}
		}

		return out;
	}

	std::string
	to_str()
		override
	{
		std::string s = "TypeName { type = \"" + type_to_str() + "\" } @ "
			+ to_hex((size_t) this);
		return s;
	}

	void
	type_check(TypeCheckState &type_check_state)
		override
	{
		if (type_check_state.classes.count(accountable_token.value))
		{
			ClassDefinition class_decl = type_check_state.classes[accountable_token.value];
			size_t byte_size           = class_decl.byte_size;

			type            = Type(Type::USER_DEFINED_CLASS, byte_size, array_sizes);
			type.class_name = accountable_token.value;

			for (const IdentifierDefinition &field : class_decl.fields)
			{
				type.fields.push_back(field.type);
			}

			return;
		}

		type = Type::from_string(accountable_token.value, array_sizes);
	}

	void
	code_gen(Assembler &assembler)
		const override
	{
	}
};

#endif