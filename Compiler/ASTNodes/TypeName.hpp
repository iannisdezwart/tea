#ifndef TEA_AST_NODE_TYPE_NAME_HEADER
#define TEA_AST_NODE_TYPE_NAME_HEADER

#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"
#include "Compiler/util.hpp"

struct TypeName final : public ASTNode
{
	std::string type_name;
	std::optional<uint32_t> class_id;
	std::vector<size_t> array_sizes;

	TypeName(CompactToken accountable_token, std::string type_name,
		std::optional<uint32_t> class_id, std::vector<size_t> &&array_sizes)
		: ASTNode(std::move(accountable_token), TYPE_NAME),
		  type_name(std::move(type_name)),
		  class_id(std::move(class_id)),
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

		out += type_name;

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
		if (class_id.has_value())
		{
			const ClassDefinition &class_def = type_check_state.classes[class_id.value()];
			size_t byte_size                 = class_def.byte_size;

			type = Type(class_id.value(), byte_size, array_sizes);

			return;
		}

		type = Type::from_string(type_name, array_sizes);
	}

	void
	code_gen(Assembler &assembler)
		const override
	{
	}
};

constexpr int TYPE_NAME_SIZE = sizeof(TypeName);

#endif