#ifndef TEA_AST_NODE_CLASS_DECLARATION_HEADER
#define TEA_AST_NODE_CLASS_DECLARATION_HEADER

#include <bits/stdc++.h>

#include "../util.hpp"
#include "ASTNode.hpp"
#include "../tokeniser.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../../Assembler/assembler.hpp"
#include "../compiler-state.hpp"
#include "TypeName.hpp"
#include "TypeIdentifierPair.hpp"
#include "FunctionDeclaration.hpp"
#include "VariableDeclaration.hpp"
#include "CodeBlock.hpp"

using namespace std;

class ClassDeclaration : public ASTNode {
	public:
		string class_name;
		vector<TypeIdentifierPair *> fields;
		vector<FunctionDeclaration *> methods;

		ClassDeclaration(
			const Token& class_token,
			const string& class_name,
			CodeBlock *body
		) : ASTNode(class_token, CLASS_DECLARATION), class_name(class_name)
		{
			for (ASTNode *node : body->statements) {
				if (node->type == FUNCTION_DECLARATION) {
					methods.push_back((FunctionDeclaration *) node);
				}

				else if (node->type == VARIABLE_DECLARATION) {
					fields.push_back(((VariableDeclaration *) node)->type_and_id_pair);
				}

				else {
					err_at_token(node->accountable_token, "Syntax Error",
						"Unexpected statement of type %s\n"
						"Only VariableDeclarations and FunctionDeclarations are allowed within a class body",
						ast_node_type_to_str(node->type));
				}
			}
		}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			for (TypeIdentifierPair *field : fields) {
				field->dfs(callback, depth + 1);
			}

			for (FunctionDeclaration *method : methods) {
				method->dfs(callback, depth + 1);
			}

			callback(this, depth);
		}

		string to_str()
		{
			string s = "ClassDeclaration { name = "
				+ class_name + " } @ " + to_hex((size_t) this);
			return s;
		}

		size_t byte_size(CompilerState& compiler_state)
		{
			size_t size = 0;

			for (TypeIdentifierPair *field : fields) {
				size += field->get_type(compiler_state).byte_size();
			}

			return size;
		}

		Type get_type(CompilerState& compiler_state)
		{
			Type type(Type::USER_DEFINED_CLASS, byte_size(compiler_state));
			type.class_name = class_name;

			for (TypeIdentifierPair *field : fields) {
				type.fields.push_back(field->get_type(compiler_state));
			}

			return type;
		}

		bool has_field(const string& field_name)
		{
			for (size_t i = 0; i < fields.size(); i++) {
				if (fields[i]->identifier_token.value == field_name) {
					return true;
				}
			}

			return false;
		}

		void define(CompilerState& compiler_state)
		{
			// Define methods

			for (FunctionDeclaration *method : methods) {
				// Add double colons to indicate scope

				Token& id_token = method->type_and_id_pair->identifier_token;
				string method_name = id_token.value;
				id_token.value = class_name + "::" + method_name;

				// Todo: check if the parameter list does not include a field name
				// Add pointer to class as first parameter (this)

				Token pointer_param_type_token;
				pointer_param_type_token.type = TYPE;
				pointer_param_type_token.value = class_name;

				Token pointer_param_id_token;
				pointer_param_id_token.type = IDENTIFIER;
				pointer_param_id_token.value = "this";

				TypeIdentifierPair *class_pointer = new TypeIdentifierPair(
						new TypeName(pointer_param_type_token, { 0 }),
						pointer_param_id_token);

				method->params.insert(method->params.begin(), 1, class_pointer);
				method->define(compiler_state);

				// Dereference field references

				method->dfs([this, method_name, &compiler_state](ASTNode *node, size_t depth) {
					if (node->type == IDENTIFIER_EXPRESSION) {
						IdentifierExpression *id_expr = (IdentifierExpression *) node;

						if (has_field(id_expr->identifier_token.value)) {
							Token this_token;
							this_token.type = IDENTIFIER;
							this_token.value = "this";

							Token op_token;
							op_token.type = OPERATOR;
							op_token.value = "->";

							IdentifierExpression *this_expr = new IdentifierExpression(this_token);
							IdentifierExpression *field_expr = new IdentifierExpression(*id_expr);
							MemberExpression *mem_expr = new MemberExpression(
								this_expr, field_expr, op_token);

							id_expr->replacement = mem_expr;
							id_expr->replacement_type = compiler_state.classes[class_name]
								.get_field_type(id_expr->identifier_token.value);
						}
					}
				}, 0);
			}
		}

		void compile(Assembler& assembler, CompilerState& compiler_state)
		{
			// Compile methods

			for (FunctionDeclaration *method : methods) {
				method->compile(assembler, compiler_state);
			}
		}
};

#endif