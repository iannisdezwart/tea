#ifndef TEA_AST_NODE_CAST_EXPRESSION_HEADER
#define TEA_AST_NODE_CAST_EXPRESSION_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "ReadValue.hpp"
#include "TypeName.hpp"
#include "../tokeniser.hpp"
#include "../util.hpp"

class CastExpression : public ReadValue {
	public:
		ReadValue *expression;
		TypeName *type_name;

		CastExpression(TypeName *type_name, ReadValue *expression)
			: type_name(type_name), expression(expression),
				ReadValue(type_name->type_token, CAST_EXPRESSION) {}

		void dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			type_name->dfs(callback, depth + 1);
			expression->dfs(callback, depth + 1);
			callback(this, depth);
		}

		std::string to_str()
		{
			std::string s = "CastExpression {} @ " + to_hex((size_t) this);
			return s;
		}

		Type get_type(CompilerState& compiler_state)
		{
			// Todo: do type checking

			if (compiler_state.classes.count(type_name->type_token.value)) {
				Class class_decl = compiler_state.classes[type_name->type_token.value];
				size_t byte_size = class_decl.byte_size;

				Type type(Type::USER_DEFINED_CLASS, byte_size, type_name->array_sizes);
				type.class_name = type_name->type_token.value;

				for (const Identifier& field : class_decl.fields) {
					type.fields.push_back(field.type);
				}

				return type;
			}

			return Type::from_string(type_name->type_token.value, type_name->array_sizes);
		}

		void get_value(Assembler &assembler, CompilerState &compiler_state, uint8_t result_reg)
		{
			expression->get_value(assembler, compiler_state, result_reg);
		}
};

#endif