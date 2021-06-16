#ifndef TEA_AST_NODE_VARIABLE_DECLARATION_HEADER
#define TEA_AST_NODE_VARIABLE_DECLARATION_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "../tokeniser.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../util.hpp"
#include "TypeIdentifierPair.hpp"
#include "InitList.hpp"

using namespace std;

class VariableDeclaration : public ASTNode {
	public:
		TypeIdentifierPair *type_and_id_pair;
		ASTNode *expression;

		VariableDeclaration(TypeIdentifierPair *type_and_id_pair,
			ASTNode *expression)
		: type_and_id_pair(type_and_id_pair), expression(expression),
			ASTNode(type_and_id_pair->identifier_token)
		{
			type = VARIABLE_DECLARATION;

			#ifdef PARSER_VERBOSE
			print("Created");
			#endif
		}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			#ifdef PARSER_VERBOSE
			print("dfs");
			#endif

			if (type_and_id_pair != NULL)
				type_and_id_pair->dfs(callback, depth + 1);

			if (expression != NULL)
				expression->dfs(callback, depth + 1);

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
						Class& cl = compiler_state.classes[type.class_name];

						// Check type compatibility

						if (init_list->items.size() > cl.fields.size()) {
							cout << init_list->accountable_token.to_str() << '\n';
							err_at_token(init_list->accountable_token, "Type Error",
								"InitList for %s class instance holds %lu members, "
								"but %s has only %lu fields\n",
								type.class_name.c_str(), init_list->items.size(),
								type.class_name.c_str(), cl.fields.size());
						}

						size_t sub_offset = 0;

						for (size_t i = 0; i < init_list->items.size(); i++) {
							ASTNode *item = init_list->items[i];
							Type item_type = item->get_type(compiler_state);
							Type& field_type = cl.fields[i].type;

							// Type compatibility

							if (!item_type.fits(field_type)) {
								// Todo: elaborate

								err_at_token(item->accountable_token, "Type Error",
									"Item %lu of InitList does not fit the corresponding field "
									"of class \"%s\"\n",
									i, type.class_name.c_str());
							}

							// Put item into class instance

							item->compile(assembler, compiler_state);

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
										"This behaviour is not implemented yet\n",
										field_type.byte_size());
							}
						}
					} else {
						err_at_token(expression->accountable_token, "Semantic Error",
							"Unexpected expression of type \"%s\" at the right hand "
							"side of a class instance declaration\n"
							"Expected an initialiser list or a constructor call\n",
							ast_node_type_to_str(expression->type));
					}
				}

				// Built-in type declaration

				else {
					// Moves result into R_ACCUMULATOR

					expression->compile(assembler, compiler_state);

					// Move R_ACCUMULATOR into the address of the variable

					switch (id_kind) {
						case IdentifierKind::LOCAL:
							switch (var_size) {
								case 1:
									assembler.move_reg_into_frame_offset_8(R_ACCUMULATOR_0_ID, offset);
									break;

								case 2:
									assembler.move_reg_into_frame_offset_16(R_ACCUMULATOR_0_ID, offset);
									break;

								case 4:
									assembler.move_reg_into_frame_offset_32(R_ACCUMULATOR_0_ID, offset);
									break;

								case 8:
									assembler.move_reg_into_frame_offset_64(R_ACCUMULATOR_0_ID, offset);
									break;

								default:
									err_at_token(accountable_token, "Invalid VariableDeclaration",
										"Cannot declare a variable with size %lu\n"
										"This behaviour is not implemented yet", var_size);
							}

							break;

						case IdentifierKind::GLOBAL:
							switch (var_size) {
								case 1:
									assembler.move_reg_into_stack_top_offset_8(R_ACCUMULATOR_0_ID, offset);
									break;

								case 2:
									assembler.move_reg_into_stack_top_offset_16(R_ACCUMULATOR_0_ID, offset);
									break;

								case 4:
									assembler.move_reg_into_stack_top_offset_32(R_ACCUMULATOR_0_ID, offset);
									break;

								case 8:
									assembler.move_reg_into_stack_top_offset_64(R_ACCUMULATOR_0_ID, offset);
									break;

								default:
									err_at_token(accountable_token, "Invalid VariableDeclaration",
										"Cannot declare a variable of with size %lu\n"
										"This behaviour is not implemented yet", var_size);
							}

							break;

						default:
							err_at_token(accountable_token, "Invalid VariableDeclaration",
								"Cannot declare a variable of this type\n"
								"Only locals and globals can be declared");
					}
				}
			}
		}
};

#endif