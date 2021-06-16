#ifndef TEA_AST_NODES_MEMBER_EXPRESSION_HEADER
#define TEA_AST_NODES_MEMBER_EXPRESSION_HEADER

#include <bits/stdc++.h>

#include "../util.hpp"
#include "ASTNode.hpp"
#include "IdentifierExpression.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../../Assembler/assembler.hpp"
#include "../compiler-state.hpp"
#include "../tokeniser.hpp"
#include "../../VM/cpu.hpp"

using namespace std;

class MemberExpression : public ASTNode {
	public:
		IdentifierExpression *object;
		IdentifierExpression *member;
		Token op_token;
		enum Operator op;

		MemberExpression(IdentifierExpression *object, IdentifierExpression *member,
			const Token& op_token)
				: object(object), member(member), op_token(op_token), ASTNode(op_token)
		{
			type = MEMBER_EXPRESSION;
			op = str_to_operator(op_token.value);

			#ifdef PARSER_VERBOSE
			print("Created");
			#endif
		}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			#ifdef PARSER_VERBOSE
			print("dfs");
			#endif

			object->dfs(callback, depth + 1);
			member->dfs(callback, depth + 1);

			callback(this, depth);
		}

		string to_str()
		{
			string s = "MemberExpression { op = \"" + to_string(op)
				+ "\" } @ " + to_hex((size_t) this);
			return s;
		}

		Type get_type(CompilerState& compiler_state)
		{
			Type object_type = object->get_type(compiler_state);
			string member_name = member->identifier_token.value;

			if (object_type != Type::USER_DEFINED_CLASS) {
				err_at_token(op_token, "Type Error",
					"Cannot access property %s from non-class type %s",
						member_name.c_str(), object->identifier_token.value.c_str());
			}

			string class_name = object_type.class_name;

			switch (op) {
				case POINTER_TO_MEMBER:
				case DEREFERENCED_POINTER_TO_MEMBER:
				{
					// Find class in compiler state

					const Class& cl = compiler_state.classes[class_name];

					for (size_t i = 0; i < cl.fields.size(); i++) {
						if (cl.fields[i].name == member_name) {
							return cl.fields[i].type;
						}
					}

					err_at_token(op_token, "Member error",
						"Class %s has no member named %s",
						class_name.c_str(), member_name.c_str());
				}

				default:
					printf("operator %s in MemberExpression not implemented\n", op_to_str(op));
					abort();
					break;
			}
		}

		void compile(Assembler& assembler, CompilerState& compiler_state)
		{
			Type object_type = object->get_type(compiler_state);
			string member_name = member->identifier_token.value;

			string class_name = object_type.class_name;
			const Class& cl = compiler_state.classes[class_name];

			// Get the offset to the member

			size_t offset = 0;

			for (size_t i = 0; i < cl.fields.size(); i++) {
				if (cl.fields[i].name == member_name) {
					break;
				}

				offset += cl.fields[i].type.byte_size();
			}

			// Get the value at the offset
			// Todo: create
		}
};

#endif