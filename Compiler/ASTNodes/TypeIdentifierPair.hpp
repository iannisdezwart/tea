#ifndef TEA_AST_NODE_TYPE_IDENTIFIER_HEADER
#define TEA_AST_NODE_TYPE_IDENTIFIER_HEADER

#include "ASTNode.hpp"
#include "../tokeniser.hpp"
#include "../compiler-state.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../util.hpp"
#include "TypeName.hpp"

struct TypeIdentifierPair : public ASTNode
{
	Token identifier_token;
	TypeName *type_name;

	TypeIdentifierPair(TypeName *type_name, const Token &identifier_token)
		: identifier_token(identifier_token), type_name(type_name),
		  ASTNode(type_name->type_token, TYPE_IDENTIFIER_PAIR) {}

	const std::string &
	get_identifier_name() const
	{
		return identifier_token.value;
	}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
	{
		type_name->dfs(callback, depth + 1);
		callback(this, depth);
	}

	std::string
	to_str()
	{
		std::string s = "TypeIdentifierPair { identifier = \"" + identifier_token.value + "\" } @ " + to_hex((size_t) this);
		return s;
	}

	Type
	get_type(CompilerState &compiler_state)
	{
		return type_name->get_type(compiler_state);
	}

	void
	compile(Assembler &assembler, CompilerState &compiler_state)
	{
	}
};

#endif