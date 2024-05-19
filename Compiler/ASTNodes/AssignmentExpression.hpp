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
#include "VM/cpu.hpp"

struct AssignmentExpression final : public ReadValue
{
	Operator op;
	std::unique_ptr<WriteValue> lhs_expr;
	std::unique_ptr<ReadValue> value;
	Type::Fits type_fits;

	AssignmentExpression(
		CompactToken accountable_token,
		Operator op,
		std::unique_ptr<WriteValue> lhs_expr,
		std::unique_ptr<ReadValue> value)
		: ReadValue(std::move(accountable_token), ASSIGNMENT_EXPRESSION),
		  op(op),
		  lhs_expr(std::move(lhs_expr)),
		  value(std::move(value)) {}

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
		type_fits = value->type.fits(lhs_expr->type);

		if (type_fits == Type::Fits::NO)
			warn("At %s, Right hand side value of AssignmentExpression does not "
			     "fit into left hand side value\n"
			     "lhs_type = %s, rhs_type = %s",
				lhs_expr->accountable_token.to_str().c_str(),
				lhs_expr->type.to_str().c_str(), value->type.to_str().c_str());

		type = lhs_expr->type;

		switch (op)
		{
		case ASSIGNMENT:
			break;
		case SUM_ASSIGNMENT:
		case DIFFERENCE_ASSIGNMENT:
		case PRODUCT_ASSIGNMENT:
		case QUOTIENT_ASSIGNMENT:
		case REMAINDER_ASSIGNMENT:
			if (!type.is_integer() && type != Type::FLOATING_POINT && type.pointer_depth() == 0)
				err_at_token(accountable_token,
					"Type Error",
					"Operator %s is not defined for type %s",
					op_to_str(op), type.to_str().c_str());
			break;
		case LEFT_SHIFT_ASSIGNMENT:
		case RIGHT_SHIFT_ASSIGNMENT:
		case BITWISE_AND_ASSIGNMENT:
		case BITWISE_OR_ASSIGNMENT:
		case BITWISE_XOR_ASSIGNMENT:
			if (!type.is_integer())
				err_at_token(accountable_token,
					"Type Error",
					"Operator %s is not defined for type %s",
					op_to_str(op), type.to_str().c_str());
			break;
		default:
			err_at_token(accountable_token,
				"Invalid AssignmentExpression",
				"Didn't find an operation to perform with operator %s",
				op_to_str(op));
		}
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

		// Implicit type casting

		if (type_fits == Type::Fits::FLT_32_TO_INT_CAST_NEEDED)
			assembler.cast_flt_32_to_int(result_reg);
		else if (type_fits == Type::Fits::FLT_64_TO_INT_CAST_NEEDED)
			assembler.cast_flt_64_to_int(result_reg);
		else if (type_fits == Type::Fits::INT_TO_FLT_32_CAST_NEEDED)
			assembler.cast_int_to_flt_32(result_reg);
		else if (type_fits == Type::Fits::INT_TO_FLT_64_CAST_NEEDED)
			assembler.cast_int_to_flt_64(result_reg);

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
		{
			if (type.is_integer() && type.byte_size() == 1)
			{
				assembler.add_int_8(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 2)
			{
				assembler.add_int_16(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 4)
			{
				assembler.add_int_32(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 8)
			{
				assembler.add_int_64(result_reg, prev_val_reg);
			}
			else if (type == Type::FLOATING_POINT && type.byte_size() == 4)
			{
				assembler.add_flt_32(result_reg, prev_val_reg);
			}
			else if (type == Type::FLOATING_POINT && type.byte_size() == 8)
			{
				assembler.add_flt_64(result_reg, prev_val_reg);
			}

			break;
		}

		case DIFFERENCE_ASSIGNMENT:
		{
			if (type.is_integer() && type.byte_size() == 1)
			{
				assembler.sub_int_8(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 2)
			{
				assembler.sub_int_16(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 4)
			{
				assembler.sub_int_32(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 8)
			{
				assembler.sub_int_64(result_reg, prev_val_reg);
			}
			else if (type == Type::FLOATING_POINT && type.byte_size() == 4)
			{
				assembler.sub_flt_32(result_reg, prev_val_reg);
			}
			else if (type == Type::FLOATING_POINT && type.byte_size() == 8)
			{
				assembler.sub_flt_64(result_reg, prev_val_reg);
			}

			break;
		}

		case PRODUCT_ASSIGNMENT:
		{
			if (type.is_integer() && type.byte_size() == 1)
			{
				assembler.mul_int_8(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 2)
			{
				assembler.mul_int_16(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 4)
			{
				assembler.mul_int_32(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 8)
			{
				assembler.mul_int_64(result_reg, prev_val_reg);
			}
			else if (type == Type::FLOATING_POINT && type.byte_size() == 4)
			{
				assembler.mul_flt_32(result_reg, prev_val_reg);
			}
			else if (type == Type::FLOATING_POINT && type.byte_size() == 8)
			{
				assembler.mul_flt_64(result_reg, prev_val_reg);
			}

			break;
		}

		case QUOTIENT_ASSIGNMENT:
		{
			if (type.is_integer() && type.byte_size() == 1)
			{
				assembler.div_int_8(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 2)
			{
				assembler.div_int_16(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 4)
			{
				assembler.div_int_32(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 8)
			{
				assembler.div_int_64(result_reg, prev_val_reg);
			}
			else if (type == Type::FLOATING_POINT && type.byte_size() == 4)
			{
				assembler.div_flt_32(result_reg, prev_val_reg);
			}
			else if (type == Type::FLOATING_POINT && type.byte_size() == 8)
			{
				assembler.div_flt_64(result_reg, prev_val_reg);
			}

			break;
		}

		case REMAINDER_ASSIGNMENT:
		{
			if (type.is_integer() && type.byte_size() == 1)
			{
				assembler.mod_int_8(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 2)
			{
				assembler.mod_int_16(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 4)
			{
				assembler.mod_int_32(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 8)
			{
				assembler.mod_int_64(result_reg, prev_val_reg);
			}

			break;
		}

		case LEFT_SHIFT_ASSIGNMENT:
		{
			if (type.is_integer() && type.byte_size() == 1)
			{
				assembler.shl_int_8(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 2)
			{
				assembler.shl_int_16(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 4)
			{
				assembler.shl_int_32(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 8)
			{
				assembler.shl_int_64(result_reg, prev_val_reg);
			}

			break;
		}

		case RIGHT_SHIFT_ASSIGNMENT:

		{
			if (type.is_integer() && type.byte_size() == 1)
			{
				assembler.shr_int_8(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 2)
			{
				assembler.shr_int_16(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 4)
			{
				assembler.shr_int_32(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 8)
			{
				assembler.shr_int_64(result_reg, prev_val_reg);
			}

			break;
		}

		case BITWISE_AND_ASSIGNMENT:
		{
			if (type.is_integer() && type.byte_size() == 1)
			{
				assembler.and_int_8(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 2)
			{
				assembler.and_int_16(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 4)
			{
				assembler.and_int_32(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 8)
			{
				assembler.and_int_64(result_reg, prev_val_reg);
			}

			break;
		}

		case BITWISE_OR_ASSIGNMENT:
		{
			if (type.is_integer() && type.byte_size() == 1)
			{
				assembler.or_int_8(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 2)
			{
				assembler.or_int_16(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 4)
			{
				assembler.or_int_32(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 8)
			{
				assembler.or_int_64(result_reg, prev_val_reg);
			}

			break;
		}

		case BITWISE_XOR_ASSIGNMENT:
		{
			if (type.is_integer() && type.byte_size() == 1)
			{
				assembler.xor_int_8(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 2)
			{
				assembler.xor_int_16(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 4)
			{
				assembler.xor_int_32(result_reg, prev_val_reg);
			}
			else if (type.is_integer() && type.byte_size() == 8)
			{
				assembler.xor_int_64(result_reg, prev_val_reg);
			}

			break;
		}

		default:
			err_at_token(accountable_token,
				"Internal Error", "Unknown assignment operator");
		}

		lhs_expr->store(assembler, prev_val_reg);
		assembler.free_register(prev_val_reg);
	}
};

constexpr int ASSIGNMENT_EXPRESSION_SIZE = sizeof(AssignmentExpression);

#endif