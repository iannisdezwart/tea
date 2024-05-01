#ifndef TEA_AST_NODE_CAST_EXPRESSION_HEADER
#define TEA_AST_NODE_CAST_EXPRESSION_HEADER

#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/ASTNodes/ReadValue.hpp"
#include "Compiler/ASTNodes/TypeName.hpp"
#include "Compiler/tokeniser.hpp"
#include "Compiler/util.hpp"

struct CastExpression final : public ReadValue
{
	std::unique_ptr<ReadValue> expression;
	std::unique_ptr<TypeName> type_name;

	CastExpression(std::unique_ptr<TypeName> type_name, std::unique_ptr<ReadValue> expression)
		: ReadValue(type_name->accountable_token, CAST_EXPRESSION),
		  expression(std::move(expression)),
		  type_name(std::move(type_name)) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
		override
	{
		type_name->dfs(callback, depth + 1);
		expression->dfs(callback, depth + 1);
		callback(this, depth);
	}

	std::string
	to_str()
		override
	{
		std::string s = "CastExpression {} @ " + to_hex((size_t) this);
		return s;
	}

	void
	type_check(TypeCheckState &type_check_state)
		override
	{
		expression->type_check(type_check_state);
		type_name->type_check(type_check_state);

		// TODO: do type checking

		if (type_check_state.classes.count(accountable_token.value))
		{
			const ClassDefinition &class_def = type_check_state.classes[accountable_token.value];
			size_t byte_size                 = class_def.byte_size;

			type            = Type(Type::USER_DEFINED_CLASS, byte_size, type_name->array_sizes);
			type.class_name = accountable_token.value;

			for (const IdentifierDefinition &field : class_def.fields)
			{
				type.fields.push_back(field.type);
			}

			return;
		}

		type = Type::from_string(accountable_token.value, type_name->array_sizes);
	}

	void
	get_value(Assembler &assembler, uint8_t result_reg)
		const override
	{
		expression->get_value(assembler, result_reg);
	}
};

#endif