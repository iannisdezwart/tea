#ifndef TEA_AST_NODES_ASSIGNMENT_EXPRESSION_HEADER
#define TEA_AST_NODES_ASSIGNMENT_EXPRESSION_HEADER

#include <bits/stdc++.h>

#include "../util.hpp"
#include "ASTNode.hpp"
#include "ReadValue.hpp"
#include "WriteValue.hpp"
#include "IdentifierExpression.hpp"
#include "MemberExpression.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../../Assembler/assembler.hpp"
#include "../compiler-state.hpp"
#include "../tokeniser.hpp"
#include "../../VM/cpu.hpp"

using namespace std;

class AssignmentExpression : public ReadValue {
	public:
		WriteValue *lhs_expr;
		ReadValue *value;
		size_t dereference_depth;
		Token op_token;
		enum Operator op;

		AssignmentExpression(
			WriteValue *lhs_expr,
			ReadValue *value,
			Token op_token,
			size_t dereference_depth = 0
		) : lhs_expr(lhs_expr), value(value), op_token(op_token),
				op(str_to_operator(op_token.value)),
				dereference_depth(dereference_depth),
				ReadValue(op_token, ASSIGNMENT_EXPRESSION) {}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			value->dfs(callback, depth + 1);
			callback(this, depth);
		}

		string to_str()
		{
			string s;

			s += "AssignmentExpression { op = \"";
			s += op_to_str(op);
			s += "\", dereference_depth = ";
			s += to_string(dereference_depth);
			s += " } @ ";
			s += to_hex((size_t) this);

			return s;
		}

		Type get_type(CompilerState& compiler_state)
		{
			Type lhs_type;

			if (lhs_expr->type == IDENTIFIER_EXPRESSION) {
				IdentifierExpression *id_expr = (IdentifierExpression *) lhs_expr;
				lhs_type = id_expr->get_type(compiler_state);
			}

			else if (lhs_expr->type == MEMBER_EXPRESSION) {
				MemberExpression *mem_expr = (MemberExpression *) lhs_expr;
				lhs_type = mem_expr->get_type(compiler_state);
			}

			else {
				err_at_token(lhs_expr->accountable_token,
					"Syntax Error",
					"Unexpected %s at the left hand side of an AssignmentExpression\n"
					"Expected an IdentifierExpression or a MemberExpression",
					ast_node_type_to_str(lhs_expr->type));
			}

			lhs_type.pointer_depth -= dereference_depth;
			Type value_type = value->get_type(compiler_state);

			if (!value_type.fits(lhs_type))
				err_at_token(lhs_expr->accountable_token,
					"Type Error",
					"Right hand side value of AssignmentExpression does not fit into "
					"left hand side value\n"
					"lhs_type = %s, rhs_type = %s",
					lhs_type.to_str().c_str(), value_type.to_str().c_str());

			return lhs_type;
		}

		bool lhs_is_id_expr()
		{
			if (lhs_expr->type == IDENTIFIER_EXPRESSION) {
				IdentifierExpression *id_expr = (IdentifierExpression *) lhs_expr;
				if (id_expr->replacement != NULL) return false;
				return true;
			} else {
				return false;
			}
		}

		MemberExpression *get_mem_expr()
		{
			if (lhs_expr->type == IDENTIFIER_EXPRESSION) {
				IdentifierExpression *id_expr = (IdentifierExpression *) lhs_expr;
				return (MemberExpression *) id_expr->replacement;
			} else {
				return (MemberExpression *) lhs_expr;
			}
		}

		/**
		 *  Performs the assignment.
		 */
		void get_value(Assembler& assembler, CompilerState& compiler_state)
		{
			size_t deref_dep = dereference_depth;
			Type var_type = get_type(compiler_state);

			if (deref_dep > 0) {
				// Moves the address of what to dereference into R_ACCUMULATOR_1

				lhs_expr->get_value(assembler, compiler_state);

				while (--deref_dep) {
					assembler.move_reg_pointer_64_into_reg(
						R_ACCUMULATOR_0_ID, R_ACCUMULATOR_0_ID);
				}

				assembler.move_reg_into_reg(R_ACCUMULATOR_0_ID, R_ACCUMULATOR_1_ID);

				// Moves the value to R_ACCUMULATOR_0

				value->get_value(assembler, compiler_state);

				switch (var_type.byte_size()) {
					case 1:
						assembler.move_reg_into_reg_pointer_8(
							R_ACCUMULATOR_0_ID, R_ACCUMULATOR_1_ID);
						break;

					case 2:
						assembler.move_reg_into_reg_pointer_16(
							R_ACCUMULATOR_0_ID, R_ACCUMULATOR_1_ID);
						break;

					case 4:
						assembler.move_reg_into_reg_pointer_32(
							R_ACCUMULATOR_0_ID, R_ACCUMULATOR_1_ID);
						break;

					case 8:
						assembler.move_reg_into_reg_pointer_64(
							R_ACCUMULATOR_0_ID, R_ACCUMULATOR_1_ID);
						break;

					default:
						printf("Dereference assignment for "
							"	byte size %lu is not implemented\n", var_type.byte_size());
						abort();
				}

				return;
			}

			// Left hand side is not a pointer
			// Moves result into R_ACCUMULATOR_0

			value->get_value(assembler, compiler_state);

			// If we're doing direct assignment,
			// directly set the variable to the new value

			if (op == ASSIGNMENT) goto move_into_var;

			// We're doing some cool assignment,
			// move the result of the expression into R_ACCUMULATOR_1_ID

			assembler.move_reg_into_reg(R_ACCUMULATOR_0_ID, R_ACCUMULATOR_1_ID);

			// Move the previous value into R_ACUMMULATOR_0_ID

			lhs_expr->get_value(assembler, compiler_state);

			// Perform the correct operation

			switch (op) {
				case SUM_ASSIGNMENT:
					assembler.add_reg_into_reg(R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
					break;

				case DIFFERENCE_ASSIGNMENT:
					assembler.subtract_reg_from_reg(R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
					break;

				case QUOTIENT_ASSIGNMENT:
					assembler.divide_reg_from_reg(R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
					break;

				case REMAINDER_ASSIGNMENT:
					assembler.take_modulo_reg_of_reg(R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
					break;

				case PRODUCT_ASSIGNMENT:
					assembler.multiply_reg_into_reg(R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
					break;

				case LEFT_SHIFT_ASSIGNMENT:
					assembler.left_shift_reg_by_reg(R_ACCUMULATOR_0_ID, R_ACCUMULATOR_1_ID);
					break;

				case RIGHT_SHIFT_ASSIGNMENT:
					assembler.right_shift_reg_by_reg(R_ACCUMULATOR_0_ID, R_ACCUMULATOR_1_ID);
					break;

				case BITWISE_AND_ASSIGNMENT:
					assembler.and_reg_into_reg(R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
					break;

				case BITWISE_XOR_ASSIGNMENT:
					assembler.xor_reg_into_reg(R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
					break;

				case BITWISE_OR_ASSIGNMENT:
					assembler.or_reg_into_reg(R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
					break;
			}

			move_into_var:

			lhs_expr->store(assembler, compiler_state);
		}
};

#endif