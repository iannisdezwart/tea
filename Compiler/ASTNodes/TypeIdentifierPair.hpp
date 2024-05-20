#ifndef TEA_AST_NODE_TYPE_IDENTIFIER_HEADER
#define TEA_AST_NODE_TYPE_IDENTIFIER_HEADER

#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/tokeniser.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/TypeName.hpp"

struct TypeIdentifierPair final : public ASTNode
{
	std::unique_ptr<TypeName> type_name;

	TypeIdentifierPair(std::unique_ptr<TypeName> type_name, Token identifier_token)
		: ASTNode(std::move(identifier_token), TYPE_IDENTIFIER_PAIR),
		  type_name(std::move(type_name)) {}

	const std::string &
	get_identifier_name() const
	{
		return accountable_token.value;
	}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
		override
	{
		type_name->dfs(callback, depth + 1);
		callback(this, depth);
	}

	std::string
	to_str()
		override
	{
		std::string s = "TypeIdentifierPair { identifier = \""
			+ accountable_token.value + "\" } @ " + to_hex((size_t) this);
		return s;
	}

	void
	type_check(TypeCheckState &type_check_state)
		override
	{
		type_name->type_check(type_check_state);
		type = type_name->type;
	}

	void
	code_gen(Assembler &assembler)
		const override
	{
	}
};

constexpr int TYPE_IDENTIFIER_PAIR_SIZE = sizeof(TypeIdentifierPair);

#endif