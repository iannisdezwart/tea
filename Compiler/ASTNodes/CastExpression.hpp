#ifndef TEA_CAST_EXPRESSION_HEADER
#define TEA_CAST_EXPRESSION_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "ReadValue.hpp"
#include "../tokeniser.hpp"
#include "../util.hpp"

class CastExpression : public ReadValue {
	public:
		ReadValue *expression;
		Token type_token;
		size_t pointer_depth;

		CastExpression(ReadValue *expression, Token type_token, size_t pointer_depth)
			: expression(expression), type_token(type_token),
				pointer_depth(pointer_depth), ReadValue(type_token, CAST_EXPRESSION) {}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			expression->dfs(callback, depth + 1);
			callback(this, depth);
		}

		string to_str()
		{
			string s = "CastExpression { type = \"" + type_token.value + "\", "
				"pointer_depth = " + to_string(pointer_depth)
				+ " } @ " + to_hex((size_t) this);
			return s;
		}

		Type get_type(CompilerState& compiler_state)
		{
			// Todo: do type checking

			if (compiler_state.classes.count(type_token.value)) {
				Class class_decl = compiler_state.classes[type_token.value];
				size_t byte_size = class_decl.byte_size;

				Type type(Type::USER_DEFINED_CLASS, byte_size, pointer_depth);
				type.class_name = type_token.value;

				for (const Identifier& field : class_decl.fields) {
					type.fields.push_back(field.type);
				}

				return type;
			}

			return Type::from_string(type_token.value);
		}

		void get_value(Assembler &assembler, CompilerState &compiler_state)
		{
			expression->get_value(assembler, compiler_state);
		}
};

#endif