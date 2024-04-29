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
#include "Compiler/ASTNodes/VariableDeclaration.hpp"
#include "Compiler/ASTNodes/CodeBlock.hpp"
#include "Compiler/ASTNodes/MemberExpression.hpp"

struct ClassDeclaration final : public ASTNode
{
	std::string class_name;
	std::vector<std::unique_ptr<VariableDeclaration>> fields;

	ClassDeclaration(
		Token class_token,
		std::string class_name,
		std::unique_ptr<CodeBlock> body)
		: ASTNode(std::move(class_token), CLASS_DECLARATION),
		  class_name(std::move(class_name))
	{
		for (std::unique_ptr<ASTNode> &node : body->statements)
		{
			if (node->node_type == VARIABLE_DECLARATION)
			{
				// TODO: As of now, variable assignments are ignored here.
				fields.push_back(static_unique_ptr_cast<VariableDeclaration>(std::move(node)));
			}

			// TODO: Support for methods has been removed.

			else
			{
				err_at_token(node->accountable_token, "Syntax Error",
					"Unexpected statement of type %s\n"
					"Only VariableDeclarations and FunctionDeclarations are allowed within a class body",
					ast_node_type_to_str(node->node_type));
			}
		}
	}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
		override
	{
		for (const std::unique_ptr<VariableDeclaration> &field : fields)
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
	type_check(TypeCheckState &type_check_state)
		override
	{
		size_t byte_size = 0;

		for (std::unique_ptr<VariableDeclaration> &field : fields)
		{
			field->type_check(type_check_state);
			byte_size += field->type.byte_size();
		}

		ClassDefinition class_def(byte_size);

		for (std::unique_ptr<VariableDeclaration> &field : fields)
		{
			class_def.add_field(field->type_and_id_pair->get_identifier_name(), field->type);
		}

		if (!type_check_state.add_class(class_name, class_def))
		{
			err_at_token(accountable_token, "Type Error",
				"Class %s has already been declared",
				class_name.c_str());
		}
	}

	void
	code_gen(Assembler &assembler)
		const override
	{
	}
};

#endif