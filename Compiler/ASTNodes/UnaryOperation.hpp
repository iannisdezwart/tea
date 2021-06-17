#ifndef TEA_AST_NODES_UNARY_OPERATION_HEADER
#define TEA_AST_NODES_UNARY_OPERATION_HEADER

#include <bits/stdc++.h>

#include "../util.hpp"
#include "ASTNode.hpp"
#include "IdentifierExpression.hpp"
#include "AssignmentExpression.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../../Assembler/assembler.hpp"
#include "../compiler-state.hpp"
#include "../tokeniser.hpp"
#include "../../VM/cpu.hpp"

using namespace std;

class UnaryOperation : public ASTNode {
	public:
		ASTNode *expression;
		Token op_token;
		bool prefix;
		enum Operator op;

		bool warned = false;

		UnaryOperation(ASTNode *expression, const Token& op_token, bool prefix)
			: expression(expression), op_token(op_token),
				prefix(prefix), ASTNode(op_token)
		{
			type = UNARY_OPERATION;
			op = str_to_operator(op_token.value, prefix);

			#ifdef PARSER_VERBOSE
			print("Created");
			#endif
		}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			#ifdef PARSER_VERBOSE
			print("dfs");
			#endif

			expression->dfs(callback, depth + 1);

			callback(this, depth);
		}

		string to_str()
		{
			string s = "UnaryOperation { op = \"" + to_string(op)
				+ "\" } @ " + to_hex((size_t) this);
			return s;
		}

		Type get_type(CompilerState& compiler_state)
		{
			Type type = expression->get_type(compiler_state);

			switch (op) {
				case POSTFIX_INCREMENT:
				case POSTFIX_DECREMENT:
				case PREFIX_INCREMENT:
				case PREFIX_DECREMENT:
				case UNARY_PLUS:
				case UNARY_MINUS:
				case BITWISE_NOT:
				case LOGICAL_NOT:
				{
					return expression->get_type(compiler_state);
				}

				case DEREFERENCE:
				{
					if (type.pointer_depth == 0)
						err_at_token(op_token, "Cannot dereference a non-pointer",
							"type %s cannot be dereferenced", type.to_str().c_str());

					type.pointer_depth--;
					return type;
				}

				case ADDRESS_OF:
				{
					type.pointer_depth++;
					return type;
				}

				default:
				{
					err_at_token(op_token, "Invalid UnaryOperation",
						"didn't find an operation to perform with operator %s",
						op_token.value.c_str());
				}
			}
		}

		void store_var(Assembler& assembler, CompilerState& compiler_state,
			Token id_token)
		{
			string id_name = id_token.value;
			IdentifierKind id_kind = compiler_state.get_identifier_kind(id_name);

			if (id_kind == IdentifierKind::UNDEFINED)
				err_at_token(id_token,
					"Reference to undeclared variable",
					"Identifier %s was referenced, but not declared",
					id_name.c_str());

			int64_t offset;
			uint64_t var_size;

			switch (id_kind) {
				case IdentifierKind::LOCAL:
				{
					Variable& var = compiler_state.locals[id_name];
					Type& type = var.id.type;
					offset = var.offset;
					var_size = type.byte_size();
					goto store_at_frame_offset;
				}

				case IdentifierKind::PARAMETER:
				{
					Variable& var = compiler_state.parameters[id_name];
					Type& type = var.id.type;
					offset = -compiler_state.parameters_size + var.offset
						- 8 - CPU::stack_frame_size;
					var_size = type.byte_size();
					goto store_at_frame_offset;
				}

				case IdentifierKind::GLOBAL:
				{
					Variable& var = compiler_state.globals[id_name];
					Type& type = var.id.type;
					offset = var.offset;
					var_size = type.byte_size();
					goto store_at_stack_top_offset;
				}

				default:
					err_at_token(id_token,
						"Identifier has unknown kind",
						"Identifier: %s. this might be a bug in the compiler",
						id_name.c_str());
			}

			// Move R_ACCUMULATOR_0 into the offset for the variable

			store_at_frame_offset:

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
			}

			store_at_stack_top_offset:

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
			}
		}

		void compile(Assembler& assembler, CompilerState& compiler_state)
		{
			switch (op) {
				case POSTFIX_INCREMENT:
				{
					// Expect an identifier as operand

					if (expression->type != IDENTIFIER_EXPRESSION)
						err_at_token(op_token, "Invalid postfix increment operand",
							"expected an identifier, found something else");

					IdentifierExpression *id_expr = (IdentifierExpression *) expression;
					Type type = id_expr->get_type(compiler_state);

					// Move result into R_ACCUMULATOR_0 and increment it

					id_expr->compile(assembler, compiler_state);

					size_t increment;

					if (type.pointer_depth > 0) {
						increment = type.pointed_byte_size();
					} else {
						increment = 1;
					}

					assembler.add_64_into_reg(increment, R_ACCUMULATOR_0_ID);

					// Store the value back into memory

					store_var(assembler, compiler_state, id_expr->identifier_token);

					// Decrement R_ACCUMULATOR_0

					assembler.subtract_64_from_reg(increment, R_ACCUMULATOR_0_ID);
					break;
				}

				case POSTFIX_DECREMENT:
				{
					// Expect an identifier as operand

					if (expression->type != IDENTIFIER_EXPRESSION)
						err_at_token(op_token, "Invalid postfix decrement operand",
							"expected an identifier, found something else");

					IdentifierExpression *id_expr = (IdentifierExpression *) expression;
					Type type = id_expr->get_type(compiler_state);

					// Move result into R_ACCUMULATOR_0 and decrement it

					id_expr->compile(assembler, compiler_state);

					size_t increment;

					if (type.pointer_depth > 0) {
						increment = type.pointed_byte_size();
					} else {
						increment = 1;
					}

					assembler.subtract_64_from_reg(increment, R_ACCUMULATOR_0_ID);

					// Store the value back into memory

					store_var(assembler, compiler_state, id_expr->identifier_token);

					// Increment R_ACCUMULATOR_0

					assembler.increment_reg(R_ACCUMULATOR_0_ID);
					break;
				}

				case PREFIX_INCREMENT:
				{
					// Expect an identifier as operand

					if (expression->type != IDENTIFIER_EXPRESSION)
						err_at_token(op_token, "Invalid postfix increment operand",
							"expected an identifier, found something else");

					IdentifierExpression *id_expr = (IdentifierExpression *) expression;
					Type type = id_expr->get_type(compiler_state);

					// Move result into R_ACCUMULATOR_0 and increment it

					id_expr->compile(assembler, compiler_state);

					size_t increment;

					if (type.pointer_depth > 0) {
						increment = type.pointed_byte_size();
					} else {
						increment = 1;
					}

					assembler.add_64_into_reg(increment, R_ACCUMULATOR_0_ID);

					// Store the value back into memory

					store_var(assembler, compiler_state, id_expr->identifier_token);
					break;
				}

				case PREFIX_DECREMENT:
				{
					// Expect an identifier as operand

					if (expression->type != IDENTIFIER_EXPRESSION)
						err_at_token(op_token, "Invalid postfix decrement operand",
							"expected an identifier, found something else");

					IdentifierExpression *id_expr = (IdentifierExpression *) expression;
					Type type = id_expr->get_type(compiler_state);

					// Move result into R_ACCUMULATOR_0 and decrement it

					id_expr->compile(assembler, compiler_state);

					size_t increment;

					if (type.pointer_depth > 0) {
						increment = type.pointed_byte_size();
					} else {
						increment = 1;
					}

					assembler.subtract_64_from_reg(increment, R_ACCUMULATOR_0_ID);

					// Store the value back into memory

					store_var(assembler, compiler_state, id_expr->identifier_token);
					break;
				}

				case UNARY_PLUS:
				{
					// Expect an identifier as operand

					if (expression->type != IDENTIFIER_EXPRESSION)
						err_at_token(op_token, "Invalid postfix decrement operand",
							"expected an identifier, found something else");

					IdentifierExpression *id_expr = (IdentifierExpression *) expression;

					// Todo: look up if this operator does anything interesting

					id_expr->compile(assembler, compiler_state);
					break;
				}

				case UNARY_MINUS:
				{
					// Expect an identifier as operand

					if (expression->type != IDENTIFIER_EXPRESSION)
						err_at_token(op_token, "Invalid postfix decrement operand",
							"expected an identifier, found something else");

					IdentifierExpression *id_expr = (IdentifierExpression *) expression;

					id_expr->compile(assembler, compiler_state);

					// Invert the bits and add one

					assembler.not_reg(R_ACCUMULATOR_0_ID);
					assembler.increment_reg(R_ACCUMULATOR_0_ID);
					break;
				}

				case BITWISE_NOT:
				{
					// Expect an identifier as operand

					if (expression->type != IDENTIFIER_EXPRESSION)
						err_at_token(op_token, "Invalid postfix decrement operand",
							"expected an identifier, found something else");

					IdentifierExpression *id_expr = (IdentifierExpression *) expression;

					id_expr->compile(assembler, compiler_state);

					// Invert the bits

					assembler.not_reg(R_ACCUMULATOR_0_ID);
					break;
				}

				case LOGICAL_NOT:
				{
					// Expect an identifier as operand

					if (expression->type != IDENTIFIER_EXPRESSION)
						err_at_token(op_token, "Invalid postfix decrement operand",
							"expected an identifier, found something else");

					IdentifierExpression *id_expr = (IdentifierExpression *) expression;

					id_expr->compile(assembler, compiler_state);

					// Invert the bits and mask

					assembler.not_reg(R_ACCUMULATOR_0_ID);
					assembler.and_64_into_reg(1, R_ACCUMULATOR_0_ID);
					break;
				}

				case DEREFERENCE:
				{
					// Moves the address of what to dereference into R_ACCUMULATOR_0

					expression->compile(assembler, compiler_state);
					Type type = expression->get_type(compiler_state);
					type.pointer_depth--;

					// Move the dereferenced value into R_ACCUMULATOR_0

					switch (type.byte_size()) {
						case 1:
							assembler.move_reg_pointer_8_into_reg(
								R_ACCUMULATOR_0_ID, R_ACCUMULATOR_0_ID);
							break;

						case 2:
							assembler.move_reg_pointer_16_into_reg(
								R_ACCUMULATOR_0_ID, R_ACCUMULATOR_0_ID);
							break;

						case 4:
							assembler.move_reg_pointer_32_into_reg(
								R_ACCUMULATOR_0_ID, R_ACCUMULATOR_0_ID);
							break;

						case 8:
							assembler.move_reg_pointer_64_into_reg(
								R_ACCUMULATOR_0_ID, R_ACCUMULATOR_0_ID);
							break;
					}

					break;
				}

				case ADDRESS_OF:
				{
					// Expect an identifier as operand

					if (expression->type != IDENTIFIER_EXPRESSION)
						err_at_token(op_token, "Invalid postfix decrement operand",
							"expected an identifier, found something else");

					IdentifierExpression *id_expr = (IdentifierExpression *) expression;

					string id_name = id_expr->identifier_token.value;
					IdentifierKind id_kind = compiler_state.get_identifier_kind(id_name);

					// Put the address of the identifier into R_ACCUMULATOR_0

					switch (id_kind) {
						case IdentifierKind::LOCAL:
						{
							Variable& var = compiler_state.locals[id_name];
							Type& type = var.id.type;
							uint64_t offset = var.offset;

							assembler.move_reg_into_reg(R_FRAME_P_ID, R_ACCUMULATOR_0_ID);
							assembler.add_64_into_reg(offset, R_ACCUMULATOR_0_ID);
							break;
						}

						case IdentifierKind::PARAMETER:
						{
							Variable& var = compiler_state.parameters[id_name];
							Type& type = var.id.type;
							uint64_t offset = compiler_state.parameters_size - var.offset
								+ 8 + CPU::stack_frame_size;

							assembler.move_reg_into_reg(R_FRAME_P_ID, R_ACCUMULATOR_0_ID);
							assembler.subtract_64_from_reg(offset, R_ACCUMULATOR_0_ID);
							break;
						}

						case IdentifierKind::GLOBAL:
						{
							Variable& var = compiler_state.globals[id_name];
							Type& type = var.id.type;
							printf("address of global is not implemented yet\n");
							abort();
							break;
						}

						default:
							err_at_token(id_expr->identifier_token,
								"Identifier has unknown kind",
								"Identifier: %s. this might be a bug in the compiler",
								id_name.c_str());
					}
				}
			}
		}
};

#endif