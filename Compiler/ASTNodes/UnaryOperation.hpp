#ifndef TEA_AST_NODES_UNARY_OPERATION_HEADER
#define TEA_AST_NODES_UNARY_OPERATION_HEADER

#include <bits/stdc++.h>

#include "../util.hpp"
#include "ASTNode.hpp"
#include "ReadValue.hpp"
#include "WriteValue.hpp"
#include "IdentifierExpression.hpp"
#include "AssignmentExpression.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../../Assembler/assembler.hpp"
#include "../compiler-state.hpp"
#include "../tokeniser.hpp"
#include "../../VM/cpu.hpp"

using namespace std;

class UnaryOperation : public WriteValue {
	public:
		ReadValue *expression;
		Token op_token;
		bool prefix;
		enum Operator op;

		bool warned = false;

		UnaryOperation(ReadValue *expression, const Token& op_token, bool prefix)
			: expression(expression), op_token(op_token),
				op(str_to_operator(op_token.value, prefix)),
				prefix(prefix), WriteValue(op_token, UNARY_OPERATION) {}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			expression->dfs(callback, depth + 1);
			callback(this, depth);
		}

		string to_str()
		{
			string s;

			s += "UnaryOperation { op = \"";
			s += op_to_str(op);
			s += "\" } @ ";
			s += to_hex((size_t) this);

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

		LocationData get_location_data(CompilerState& compiler_state)
		{
			// Todo: create
		}

		void store(Assembler& assembler, CompilerState& compiler_state)
		{
			switch (op) {
				case PREFIX_INCREMENT:
				{
					WriteValue *wr_expression = WriteValue::cast(expression);
					wr_expression->store(assembler, compiler_state);
					break;
				}

				case PREFIX_DECREMENT:
				{
					WriteValue *wr_expression = WriteValue::cast(expression);
					wr_expression->store(assembler, compiler_state);
					break;
				}

				case DEREFERENCE:
				{
					size_t deref_dep = 0;
					ReadValue *expr = this;

					while (
						expr->type == UNARY_OPERATION
						&& ((UnaryOperation *) expr)->op == DEREFERENCE
					) {
						expr = ((UnaryOperation *) expr)->expression;
						deref_dep++;
					}

					// Move the value to store into R_ACCUMULATOR_1

					assembler.move_reg_into_reg(R_ACCUMULATOR_0_ID, R_ACCUMULATOR_1_ID);

					// Move the address of what to dereference into R_ACCUMULATOR_0

					expr->get_value(assembler, compiler_state);

					Type expr_type = expr->get_type(compiler_state);
					expr_type.pointer_depth -= deref_dep;

					// Dereference

					while (--deref_dep) {
						assembler.move_reg_pointer_64_into_reg(
							R_ACCUMULATOR_0_ID, R_ACCUMULATOR_0_ID);
					}

					switch (expr_type.byte_size()) {
						case 1:
							assembler.move_reg_into_reg_pointer_8(
								R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
							break;

						case 2:
							assembler.move_reg_into_reg_pointer_16(
								R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
							break;

						case 4:
							assembler.move_reg_into_reg_pointer_32(
							R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
							break;

						case 8:
							assembler.move_reg_into_reg_pointer_64(
								R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
							break;

						default:
							printf("Dereference assignment for "
								"	byte size %lu is not implemented\n", 
								expr->get_type(compiler_state).byte_size());
							abort();
					}

					break;
				}

				default:
					err_at_token(op_token,
						"Value Type Error",
						"Expected a WriteValue\n"
						"This value is not writable");
			}
		}

		void get_value(Assembler& assembler, CompilerState& compiler_state)
		{
			switch (op) {
				case POSTFIX_INCREMENT:
				{
					WriteValue *wr_expression = WriteValue::cast(expression);
					Type type = wr_expression->get_type(compiler_state);

					// Move result into R_ACCUMULATOR_0 and increment it

					wr_expression->get_value(assembler, compiler_state);

					if (type.pointer_depth > 0) {
						assembler.add_64_into_reg(type.pointed_byte_size(), R_ACCUMULATOR_0_ID);
					} else {
						assembler.increment_reg(R_ACCUMULATOR_0_ID);
					}

					// Store the value back into memory

					wr_expression->store(assembler, compiler_state);

					// Decrement R_ACCUMULATOR_0

					if (type.pointer_depth > 0) {
						assembler.subtract_64_from_reg(type.pointed_byte_size(), R_ACCUMULATOR_0_ID);
					} else {
						assembler.decrement_reg(R_ACCUMULATOR_0_ID);
					}

					break;
				}

				case POSTFIX_DECREMENT:
				{
					WriteValue *wr_expression = WriteValue::cast(expression);
					Type type = wr_expression->get_type(compiler_state);

					// Move result into R_ACCUMULATOR_0 and decrement it

					wr_expression->get_value(assembler, compiler_state);

					if (type.pointer_depth > 0) {
						assembler.subtract_64_from_reg(type.pointed_byte_size(), R_ACCUMULATOR_0_ID);
					} else {
						assembler.decrement_reg(R_ACCUMULATOR_0_ID);
					}

					// Store the value back into memory

					wr_expression->store(assembler, compiler_state);

					// Increment R_ACCUMULATOR_0

					if (type.pointer_depth > 0) {
						assembler.add_64_into_reg(type.pointed_byte_size(), R_ACCUMULATOR_0_ID);
					} else {
						assembler.increment_reg(R_ACCUMULATOR_0_ID);
					}

					break;
				}

				case PREFIX_INCREMENT:
				{
					WriteValue *wr_expression = WriteValue::cast(expression);
					Type type = wr_expression->get_type(compiler_state);

					// Move result into R_ACCUMULATOR_0 and increment it

					wr_expression->get_value(assembler, compiler_state);

					wr_expression->get_value(assembler, compiler_state);

					if (type.pointer_depth > 0) {
						assembler.add_64_into_reg(type.pointed_byte_size(), R_ACCUMULATOR_0_ID);
					} else {
						assembler.increment_reg(R_ACCUMULATOR_0_ID);
					}

					// Store the value back into memory

					wr_expression->store(assembler, compiler_state);
					break;
				}

				case PREFIX_DECREMENT:
				{
					WriteValue *wr_expression = WriteValue::cast(expression);
					Type type = wr_expression->get_type(compiler_state);

					// Move result into R_ACCUMULATOR_0 and decrement it

					wr_expression->get_value(assembler, compiler_state);

					if (type.pointer_depth > 0) {
						assembler.subtract_64_from_reg(type.pointed_byte_size(), R_ACCUMULATOR_0_ID);
					} else {
						assembler.decrement_reg(R_ACCUMULATOR_0_ID);
					}

					// Store the value back into memory

					wr_expression->store(assembler, compiler_state);
					break;
				}

				case UNARY_PLUS:
				{
					// Todo: look up if this operator does anything interesting

					expression->get_value(assembler, compiler_state);
					break;
				}

				case UNARY_MINUS:
				{
					expression->get_value(assembler, compiler_state);

					// Invert the bits and add one

					assembler.not_reg(R_ACCUMULATOR_0_ID);
					assembler.increment_reg(R_ACCUMULATOR_0_ID);
					break;
				}

				case BITWISE_NOT:
				{
					expression->get_value(assembler, compiler_state);

					// Invert the bits

					assembler.not_reg(R_ACCUMULATOR_0_ID);
					break;
				}

				case LOGICAL_NOT:
				{
					expression->get_value(assembler, compiler_state);

					// Invert the bits and mask

					assembler.not_reg(R_ACCUMULATOR_0_ID);
					assembler.and_64_into_reg(1, R_ACCUMULATOR_0_ID);
					break;
				}

				case DEREFERENCE:
				{
					// Moves the address of what to dereference into R_ACCUMULATOR_0

					expression->get_value(assembler, compiler_state);
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
					WriteValue *wr_expression = WriteValue::cast(expression);
					LocationData location_data = wr_expression->get_location_data(compiler_state);

					if (location_data.is_at_frame_top()) {
						assembler.move_reg_into_reg(R_FRAME_P_ID, R_ACCUMULATOR_0_ID);
						assembler.add_64_into_reg(location_data.offset, R_ACCUMULATOR_0_ID);
					}

					else {
						assembler.move_stack_top_address_into_reg(R_ACCUMULATOR_0_ID);
						assembler.add_64_into_reg(location_data.offset, R_ACCUMULATOR_0_ID);
					}
				}
			}
		}
};

#endif