#ifndef TEA_AST_NODE_CLASS_DECLARATION_HEADER
#define TEA_AST_NODE_CLASS_DECLARATION_HEADER

#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/tokeniser.hpp"
#include "Compiler/code-gen/Assembler.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"
#include "Compiler/ASTNodes/TypeName.hpp"
#include "Compiler/ASTNodes/TypeIdentifierPair.hpp"
#include "Compiler/ASTNodes/FunctionDeclaration.hpp"
#include "Compiler/ASTNodes/CodeBlock.hpp"
#include "Compiler/ASTNodes/MemberExpression.hpp"

struct ClassDeclaration final : public ASTNode
{
	std::string class_name;
	std::vector<std::unique_ptr<TypeIdentifierPair>> fields;

	ClassDeclaration(
		Token class_token,
		std::string class_name,
		std::vector<std::unique_ptr<TypeIdentifierPair>> &&fields)
		: ASTNode(std::move(class_token), CLASS_DECLARATION),
		  class_name(std::move(class_name)), fields(std::move(fields)) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
		override
	{
		for (const std::unique_ptr<TypeIdentifierPair> &field : fields)
		{
			field->dfs(callback, depth + 1);
		}

		callback(this, depth);
	}

	std::string
	to_str()
		override
	{
		std::string s = "ClassDeclaration { name = "
			+ class_name + " } @ " + to_hex((size_t) this);
		return s;
	}

	void
	pre_type_check(TypeCheckState &type_check_state)
		override
	{
		if (!type_check_state.def_class(class_name))
		{
			err_at_token(accountable_token, "Type Error",
				"Class %s has already been declared",
				class_name.c_str());
		}
	}

	void
	type_check(TypeCheckState &type_check_state)
		override
	{
		size_t byte_size = 0;

		for (std::unique_ptr<TypeIdentifierPair> &field : fields)
		{
			field->type_check(type_check_state);
			byte_size += field->type.byte_size();
		}

		ClassDefinition class_def(byte_size);

		for (std::unique_ptr<TypeIdentifierPair> &field : fields)
		{
			class_def.add_field(field->get_identifier_name(), field->type);
		}

		type_check_state.add_class(class_name, class_def);
	}

	void
	code_gen(Assembler &assembler)
		const override
	{
	}
};

#endif