#ifndef TEA_AST_NODE_VARIABLE_DECLARATION_HEADER
#define TEA_AST_NODE_VARIABLE_DECLARATION_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "ReadValue.hpp"
#include "IdentifierExpression.hpp"
#include "../tokeniser.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../util.hpp"
#include "TypeIdentifierPair.hpp"
#include "InitList.hpp"

using namespace std;

class VariableDeclaration : public ASTNode {
	public:
		TypeIdentifierPair *type_and_id_pair;
		IdentifierExpression id_expr;
		ReadValue *expression;

		VariableDeclaration(TypeIdentifierPair *type_and_id_pair,
			ReadValue *expression)
		: type_and_id_pair(type_and_id_pair),
			id_expr(type_and_id_pair->identifier_token), expression(expression),
			ASTNode(type_and_id_pair->identifier_token, VARIABLE_DECLARATION) {}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			if (type_and_id_pair != NULL) type_and_id_pair->dfs(callback, depth + 1);
			if (expression != NULL) expression->dfs(callback, depth + 1);

			callback(this, depth);
		}

		string to_str()
		{
			string s = "VariableDeclaration {} @ " + to_hex((size_t) this);
			return s;
		}

		Type get_type(CompilerState& compiler_state)
		{
			return type_and_id_pair->get_type(compiler_state);
		}

		void compile(Assembler& assembler, CompilerState& compiler_state)
		{
			if (expression != NULL) {
				// Match types

				Type specified_type = get_type(compiler_state);
				Type assignment_type = expression->get_type(compiler_state);

				if (!assignment_type.fits(specified_type)) {
					err_at_token(expression->accountable_token,
						"Type Error",
						"Initial value of VariableDeclaration does not fit into "
						"specified type\n"
						"specified_type = %s, assignment_value = %s",
						specified_type.to_str().c_str(), assignment_type.to_str().c_str());
				}

				string id_name = type_and_id_pair->get_identifier_name();
				IdentifierKind id_kind = compiler_state.get_identifier_kind(id_name);

				Variable var;

				switch (id_kind) {
					case IdentifierKind::LOCAL:
						var = compiler_state.locals[id_name];
						break;

					case IdentifierKind::GLOBAL:
						var = compiler_state.globals[id_name];
						break;

					default:
						err_at_token(accountable_token, "Invalid VariableDeclaration",
							"Cannot declare a variable of this type\n"
							"Only locals and globals can be declared");
				}

				Type& type = var.id.type;
				uint64_t offset = var.offset;
				uint64_t var_size = type.byte_size();

				// Class instance declaration

				if (type == Type::USER_DEFINED_CLASS) {
					// Expect an init list or a constructor call

					if (expression->type == INIT_LIST) {
						InitList *init_list = (InitList *) expression;
						const Class& cl = compiler_state.classes[type.class_name];

						// Check type compatibility

						if (init_list->items.size() > cl.fields.size()) {
							err_at_token(init_list->accountable_token, "Type Error",
								"InitList for %s class instance holds %lu members, "
								"but %s has only %lu fields",
								type.class_name.c_str(), init_list->items.size(),
								type.class_name.c_str(), cl.fields.size());
						}

						size_t sub_offset = 0;

						for (size_t i = 0; i < init_list->items.size(); i++) {
							ASTNode *item = init_list->items[i];
							const Type item_type = item->get_type(compiler_state);
							const Type& field_type = cl.fields[i].type;

							// Type compatibility

							if (!item_type.fits(field_type)) {
								// Todo: elaborate

								err_at_token(item->accountable_token, "Type Error",
									"Item %lu of InitList does not fit the corresponding field "
									"of class \"%s\"",
									i, type.class_name.c_str());
							}

							// Put item into class instance

							item->compile(assembler, compiler_state);

							switch (id_kind) {
								case IdentifierKind::LOCAL:
								{
									switch (field_type.byte_size()) {
										case 1:
											assembler.move_reg_into_frame_offset_8(R_ACCUMULATOR_0_ID, offset + sub_offset);
											sub_offset += 1;
											break;

										case 2:
											assembler.move_reg_into_frame_offset_16(R_ACCUMULATOR_0_ID, offset + sub_offset);
											sub_offset += 2;
											break;

										case 4:
											assembler.move_reg_into_frame_offset_32(R_ACCUMULATOR_0_ID, offset + sub_offset);
											sub_offset += 4;
											break;

										case 8:
											assembler.move_reg_into_frame_offset_64(R_ACCUMULATOR_0_ID, offset + sub_offset);
											sub_offset += 8;
											break;

										default:
											err_at_token(item->accountable_token, "Type Error",
												"Cannot put an item of %lu bytes into a class instance\n"
												"This behaviour is not implemented yet",
												field_type.byte_size());
									}

									break;
								}

								case IdentifierKind::GLOBAL:
								{
									switch (field_type.byte_size()) {
										case 1:
											assembler.move_reg_into_stack_top_offset_8(R_ACCUMULATOR_0_ID, offset + sub_offset);
											sub_offset += 1;
											break;

										case 2:
											assembler.move_reg_into_stack_top_offset_16(R_ACCUMULATOR_0_ID, offset + sub_offset);
											sub_offset += 2;
											break;

										case 4:
											assembler.move_reg_into_stack_top_offset_32(R_ACCUMULATOR_0_ID, offset + sub_offset);
											sub_offset += 4;
											break;

										case 8:
											assembler.move_reg_into_stack_top_offset_64(R_ACCUMULATOR_0_ID, offset + sub_offset);
											sub_offset += 8;
											break;

										default:
											err_at_token(item->accountable_token, "Type Error",
												"Cannot put an item of %lu bytes into a class instance\n"
												"This behaviour is not implemented yet",
												field_type.byte_size());
									}

									break;
								}
							}

						}
					} else {
						err_at_token(expression->accountable_token, "Semantic Error",
							"Unexpected expression of type \"%s\" at the right hand "
							"side of a class instance declaration\n"
							"Expected an initialiser list or a constructor call",
							ast_node_type_to_str(expression->type));
					}

					return;
				}

				// Built-in type declaration

				// Moves result into R_ACCUMULATOR_0

				expression->get_value(assembler, compiler_state);

				// Move R_ACCUMULATOR into the address of the variable

				id_expr.store(assembler, compiler_state);
			}
		}
};

#endif