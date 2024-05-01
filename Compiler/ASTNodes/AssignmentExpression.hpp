#ifndef TEA_AST_NODES_ASSIGNMENT_EXPRESSION_HEADER
#define TEA_AST_NODES_ASSIGNMENT_EXPRESSION_HEADER

#include <functional>

#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/ASTNodes/ReadValue.hpp"
#include "Compiler/ASTNodes/WriteValue.hpp"
#include "Compiler/ASTNodes/IdentifierExpression.hpp"
#include "Compiler/ASTNodes/MemberExpression.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/code-gen/Assembler.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"
#include "Compiler/tokeniser.hpp"
#include "VM/cpu.hpp"

struct AssignmentExpression final : public ReadValue
{
	std::unique_ptr<WriteValue> lhs_expr;
	std::unique_ptr<ReadValue> value;
	Operator op;

	AssignmentExpression(
		std::unique_ptr<WriteValue> lhs_expr,
		std::unique_ptr<ReadValue> value,
		Token op_token)
		: ReadValue(std::move(op_token), ASSIGNMENT_EXPRESSION),
		  lhs_expr(std::move(lhs_expr)),
		  value(std::move(value)),
		  op(str_to_operator(accountable_token.value)) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
		override
	{
		value->dfs(callback, depth + 1);
		lhs_expr->dfs(callback, depth + 1);
		callback(this, depth);
	}

	std::string
	to_str()
		override
	{
		std::string s;

		s += "AssignmentExpression { op = \"";
		s += op_to_str(op);
		s += "\" } @ ";
		s += to_hex((size_t) this);

		return s;
	}

	void
	type_check(TypeCheckState &type_check_state)
		override
	{
		lhs_expr->type_check(type_check_state);
		value->type_check(type_check_state);

		if (!value->type.fits(lhs_expr->type))
			err_at_token(lhs_expr->accountable_token,
				"Type Error",
				"Right hand side value of AssignmentExpression does not fit into "
				"left hand side value\n"
				"lhs_type = %s, rhs_type = %s",
				lhs_expr->type.to_str().c_str(), value->type.to_str().c_str());

		type = lhs_expr->type;
	}

	/**
	 *  Performs the assignment.
	 */
	void
	get_value(Assembler &assembler, uint8_t result_reg)
		const override
	{
		// Moves result into its register

		value->get_value(assembler, result_reg);

		// If we're doing direct assignment,
		// directly set the variable to the new value

		if (op == ASSIGNMENT)
		{
			lhs_expr->store(assembler, result_reg);
			return;
		}

		// We're doing some cool assignment,
		// move the previous value into its register

		uint8_t prev_val_reg = assembler.get_register();
		lhs_expr->get_value(assembler, prev_val_reg);

		// Perform the correct operation

		switch (op)
		{
		case SUM_ASSIGNMENT:
			assembler.add_reg_into_reg(result_reg, prev_val_reg);
			break;

		case DIFFERENCE_ASSIGNMENT:
			assembler.subtract_reg_from_reg(result_reg, prev_val_reg);
			break;

		case QUOTIENT_ASSIGNMENT:
			assembler.divide_reg_from_reg(result_reg, prev_val_reg);
			break;

		case REMAINDER_ASSIGNMENT:
			assembler.take_modulo_reg_of_reg(result_reg, prev_val_reg);
			break;

		case PRODUCT_ASSIGNMENT:
			assembler.multiply_reg_into_reg(result_reg, prev_val_reg);
			break;

		case LEFT_SHIFT_ASSIGNMENT:
			assembler.left_shift_reg_by_reg(prev_val_reg, result_reg);
			break;

		case RIGHT_SHIFT_ASSIGNMENT:
			assembler.right_shift_reg_by_reg(prev_val_reg, result_reg);
			break;

		case BITWISE_AND_ASSIGNMENT:
			assembler.and_reg_into_reg(result_reg, prev_val_reg);
			break;

		case BITWISE_XOR_ASSIGNMENT:
			assembler.xor_reg_into_reg(result_reg, prev_val_reg);
			break;

		case BITWISE_OR_ASSIGNMENT:
			assembler.or_reg_into_reg(result_reg, prev_val_reg);
			break;

		default:
			err_at_token(accountable_token,
				"Internal Error", "Unknown assignment operator");
		}

		lhs_expr->store(assembler, prev_val_reg);
		assembler.free_register(prev_val_reg);
	}
};

#endif