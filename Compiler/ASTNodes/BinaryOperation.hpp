#ifndef TEA_AST_NODE_BINARY_OPERATION_HEADER
#define TEA_AST_NODE_BINARY_OPERATION_HEADER

#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/ASTNodes/ReadValue.hpp"
#include "Compiler/code-gen/Assembler.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"
#include "Compiler/tokeniser.hpp"

struct BinaryOperation final : public ReadValue
{
	std::unique_ptr<ReadValue> left;
	std::unique_ptr<ReadValue> right;
	Operator op;
	Type cmp_type;

	BinaryOperation(std::unique_ptr<ReadValue> left, std::unique_ptr<ReadValue> right, Token op_token)
		: ReadValue(std::move(op_token), BINARY_OPERATION),
		  left(std::move(left)),
		  right(std::move(right)),
		  op(str_to_operator(accountable_token.value)) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
		override
	{
		left->dfs(callback, depth + 1);
		right->dfs(callback, depth + 1);

		callback(this, depth);
	}

	std::string
	to_str()
		override
	{
		std::string s;

		s += "BinaryOperation { op = \"";
		s += op_to_str(op);
		s += "\" } @ ";
		s += to_hex((size_t) this);

		return s;
	}

	void
	type_check(TypeCheckState &type_check_state)
		override
	{
		left->type_check(type_check_state);
		right->type_check(type_check_state);

		size_t left_size  = left->type.byte_size();
		size_t right_size = right->type.byte_size();

		switch (op)
		{
		case ADDITION:
		case SUBTRACTION:
		{
			// sizeX + sizeY -> max(sizeX, sizeY)

			if (left->type.pointer_depth() == 0
				&& right->type.pointer_depth() == 0)
			{
				if (left->type == Type::FLOATING_POINT && right->type != Type::FLOATING_POINT)
				{
					type = left->type;
					return;
				}

				if (right->type == Type::FLOATING_POINT && left->type != Type::FLOATING_POINT)
				{
					type = right->type;
					return;
				}

				if (left_size > right_size)
				{
					type = left->type;
					return;
				}

				type = right->type;
				return;
			}

			if (!left->type.is_integer() || !right->type.is_integer())
			{
				warn("operator %s (%s) is not implemented for types x = %s and y = %s\n"
				     "At %ld:%ld\n",
					op_to_str(op), op_to_example_str(op),
					left->type.to_str().c_str(),
					right->type.to_str().c_str(), accountable_token.line,
					accountable_token.col);
			}

			// pointer + int -> pointer

			if (left->type.pointer_depth() > 0
				&& right->type.pointer_depth() == 0)
			{
				type = left->type;
				return;
			}

			// int + pointer -> pointer

			if (right->type.pointer_depth() > 0
				&& left->type.pointer_depth() == 0)
			{
				type = right->type;
				return;
			}

			break;
		}

		case MULTIPLICATION:
		case DIVISION:
		{
			// sizeX * sizeY -> max(sizeX, sizeY)

			if (left->type.pointer_depth() == 0
				&& right->type.pointer_depth() == 0)
			{
				if (left->type == Type::FLOATING_POINT && right->type != Type::FLOATING_POINT)
				{
					type = left->type;
					return;
				}

				if (right->type == Type::FLOATING_POINT && left->type != Type::FLOATING_POINT)
				{
					type = right->type;
					return;
				}

				if (left_size > right_size)
				{
					type = left->type;
					return;
				}

				type = right->type;
				return;
			}

			break;
		}

		case REMAINDER:
		{
			if (!left->type.is_integer() || !right->type.is_integer())
			{
				warn("operator %s (%s) is not implemented for types x = %s and y = %s\n"
				     "At %ld:%ld\n",
					op_to_str(op), op_to_example_str(op),
					left->type.to_str().c_str(),
					right->type.to_str().c_str(), accountable_token.line,
					accountable_token.col);
			}

			// intX % intY -> intX

			if (left->type.pointer_depth() == 0
				&& right->type.pointer_depth() == 0)
			{
				type = left->type;
				return;
			}
		}

		case BITWISE_AND:
		case BITWISE_XOR:
		case BITWISE_OR:
		case LEFT_SHIFT:
		case RIGHT_SHIFT:
		{
			if (!left->type.is_integer() || !right->type.is_integer())
			{
				warn("operator %s (%s) is not implemented for types x = %s and y = %s\n"
				     "At %ld:%ld\n",
					op_to_str(op), op_to_example_str(op),
					left->type.to_str().c_str(),
					right->type.to_str().c_str(), accountable_token.line,
					accountable_token.col);
			}

			// intX & intX

			if (left->type.pointer_depth() == 0
				&& right->type.pointer_depth() == 0
				&& left_size == right_size)
			{
				type = left->type;
				return;
			}
		}

		case LESS:
		case LESS_OR_EQUAL:
		case GREATER:
		case GREATER_OR_EQUAL:
		case EQUAL:
		case NOT_EQUAL:
		{
			if (left->type != right->type)
			{
				warn("comparing different types x = %s and y = %s\n"
				     "At %ld:%ld\n",
					left->type.to_str().c_str(),
					right->type.to_str().c_str(), accountable_token.line,
					accountable_token.col);
			}

			type = Type(left->type, 1); // Result of comparison is boolean

			if (left->type.pointer_depth() == 0
				&& right->type.pointer_depth() == 0)
			{
				if (left->type == Type::FLOATING_POINT && right->type != Type::FLOATING_POINT)
				{
					cmp_type = left->type;
					return;
				}

				if (right->type == Type::FLOATING_POINT && left->type != Type::FLOATING_POINT)
				{
					cmp_type = right->type;
					return;
				}

				if (left_size > right_size)
				{
					cmp_type = left->type;
					return;
				}

				cmp_type = right->type;
				return;
			}
			else
			{
				cmp_type = left->type;
			}

			return;
		}

		default:
			p_warn(stderr, "operator %s in BinaryOperation not implemented\n",
				op_to_str(op));
			abort();
			break;
		}

		// Warn the user that this operator is not implemented
		// for the given types and undefined behaviour
		// might occur. This should be removed in the future
		// and replaced by a compile-time error
		// when all operators are properly implemented.

		warn("operator %s (%s) is not implemented for types x = %s and y = %s "
		     "and might cause undefined behaviour\n"
		     "At %ld:%ld\n",
			op_to_str(op), op_to_example_str(op),
			left->type.to_str().c_str(),
			right->type.to_str().c_str(), accountable_token.line,
			accountable_token.col);

		type = left->type;
	}

	void
	get_value(Assembler &assembler, uint8_t result_reg)
		const override
	{
		uint8_t rhs_reg;

		// Get the left hand side value.

		left->get_value(assembler, result_reg);

		// Get the right hand side value.

		rhs_reg = assembler.get_register();
		right->get_value(assembler, rhs_reg);

		// Implicit type casting

		Type::Fits left_fits  = left->type.fits(type);
		Type::Fits right_fits = right->type.fits(type);

		if (left_fits == Type::Fits::FLT_32_TO_INT_CAST_NEEDED)
			assembler.cast_flt_32_to_int(result_reg);
		else if (left_fits == Type::Fits::FLT_64_TO_INT_CAST_NEEDED)
			assembler.cast_flt_64_to_int(result_reg);
		else if (left_fits == Type::Fits::INT_TO_FLT_32_CAST_NEEDED)
			assembler.cast_int_to_flt_32(result_reg);
		else if (left_fits == Type::Fits::INT_TO_FLT_64_CAST_NEEDED)
			assembler.cast_int_to_flt_64(result_reg);

		if (right_fits == Type::Fits::FLT_32_TO_INT_CAST_NEEDED)
			assembler.cast_flt_32_to_int(rhs_reg);
		else if (right_fits == Type::Fits::FLT_64_TO_INT_CAST_NEEDED)
			assembler.cast_flt_64_to_int(rhs_reg);
		else if (right_fits == Type::Fits::INT_TO_FLT_32_CAST_NEEDED)
			assembler.cast_int_to_flt_32(rhs_reg);
		else if (right_fits == Type::Fits::INT_TO_FLT_64_CAST_NEEDED)
			assembler.cast_int_to_flt_64(rhs_reg);

		// Perform the operation.

		auto compare_and_set = [&](std::function<void()> set_if_cb)
		{
			if (cmp_type == Type::SIGNED_INTEGER && cmp_type.byte_size() == 1)
			{
				assembler.cmp_int_8(result_reg, rhs_reg);
			}
			else if (cmp_type == Type::UNSIGNED_INTEGER && cmp_type.byte_size() == 1)
			{
				assembler.cmp_int_8_u(result_reg, rhs_reg);
			}
			if (cmp_type == Type::SIGNED_INTEGER && cmp_type.byte_size() == 2)
			{
				assembler.cmp_int_16(result_reg, rhs_reg);
			}
			else if (cmp_type == Type::UNSIGNED_INTEGER && cmp_type.byte_size() == 2)
			{
				assembler.cmp_int_16_u(result_reg, rhs_reg);
			}
			if (cmp_type == Type::SIGNED_INTEGER && cmp_type.byte_size() == 4)
			{
				assembler.cmp_int_32(result_reg, rhs_reg);
			}
			else if (cmp_type == Type::UNSIGNED_INTEGER && cmp_type.byte_size() == 4)
			{
				assembler.cmp_int_32_u(result_reg, rhs_reg);
			}
			if (cmp_type == Type::SIGNED_INTEGER && cmp_type.byte_size() == 8)
			{
				assembler.cmp_int_64(result_reg, rhs_reg);
			}
			else if (cmp_type == Type::UNSIGNED_INTEGER && cmp_type.byte_size() == 8)
			{
				assembler.cmp_int_64_u(result_reg, rhs_reg);
			}
			else if (cmp_type == Type::FLOATING_POINT && cmp_type.byte_size() == 4)
			{
				assembler.cmp_flt_32(result_reg, rhs_reg);
			}
			else if (cmp_type == Type::FLOATING_POINT && cmp_type.byte_size() == 8)
			{
				assembler.cmp_flt_64(result_reg, rhs_reg);
			}
			set_if_cb();
		};

		switch (op)
		{
			// Mathematical binary operations.

		case DIVISION:
		{
			if (type.is_integer() && type.byte_size() == 1)
			{
				assembler.div_int_8(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 2)
			{
				assembler.div_int_16(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 4)
			{
				assembler.div_int_32(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 8)
			{
				assembler.div_int_64(rhs_reg, result_reg);
			}
			else if (type == Type::FLOATING_POINT && type.byte_size() == 4)
			{
				assembler.div_flt_32(rhs_reg, result_reg);
			}
			else if (type == Type::FLOATING_POINT && type.byte_size() == 8)
			{
				assembler.div_flt_64(rhs_reg, result_reg);
			}

			break;
		}

		case REMAINDER:
		{
			if (type.is_integer() && type.byte_size() == 1)
			{
				assembler.mod_int_8(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 2)
			{
				assembler.mod_int_16(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 4)
			{
				assembler.mod_int_32(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 8)
			{
				assembler.mod_int_64(rhs_reg, result_reg);
			}

			break;
		}

		case MULTIPLICATION:
		{
			if (type.is_integer() && type.byte_size() == 1)
			{
				assembler.mul_int_8(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 2)
			{
				assembler.mul_int_16(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 4)
			{
				assembler.mul_int_32(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 8)
			{
				assembler.mul_int_64(rhs_reg, result_reg);
			}
			else if (type == Type::FLOATING_POINT && type.byte_size() == 4)
			{
				assembler.mul_flt_32(rhs_reg, result_reg);
			}
			else if (type == Type::FLOATING_POINT && type.byte_size() == 8)
			{
				assembler.mul_flt_64(rhs_reg, result_reg);
			}

			break;
		}

		case ADDITION:
		{
			if (type.is_integer() && type.byte_size() == 1)
			{
				assembler.add_int_8(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 2)
			{
				assembler.add_int_16(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 4)
			{
				assembler.add_int_32(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 8)
			{
				assembler.add_int_64(rhs_reg, result_reg);
			}
			else if (type == Type::FLOATING_POINT && type.byte_size() == 4)
			{
				assembler.add_flt_32(rhs_reg, result_reg);
			}
			else if (type == Type::FLOATING_POINT && type.byte_size() == 8)
			{
				assembler.add_flt_64(rhs_reg, result_reg);
			}

			break;
		}

		case SUBTRACTION:
		{
			if (type.is_integer() && type.byte_size() == 1)
			{
				assembler.sub_int_8(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 2)
			{
				assembler.sub_int_16(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 4)
			{
				assembler.sub_int_32(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 8)
			{
				assembler.sub_int_64(rhs_reg, result_reg);
			}
			else if (type == Type::FLOATING_POINT && type.byte_size() == 4)
			{
				assembler.sub_flt_32(rhs_reg, result_reg);
			}
			else if (type == Type::FLOATING_POINT && type.byte_size() == 8)
			{
				assembler.sub_flt_64(rhs_reg, result_reg);
			}

			break;
		}

		case BITWISE_AND:
		{
			if (type.is_integer() && type.byte_size() == 1)
			{
				assembler.and_int_8(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 2)
			{
				assembler.and_int_16(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 4)
			{
				assembler.and_int_32(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 8)
			{
				assembler.and_int_64(rhs_reg, result_reg);
			}

			break;
		}

		case BITWISE_XOR:
		{
			if (type.is_integer() && type.byte_size() == 1)
			{
				assembler.xor_int_8(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 2)
			{
				assembler.xor_int_16(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 4)
			{
				assembler.xor_int_32(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 8)
			{
				assembler.xor_int_64(rhs_reg, result_reg);
			}

			break;
		}

		case BITWISE_OR:
		{
			if (type.is_integer() && type.byte_size() == 1)
			{
				assembler.or_int_8(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 2)
			{
				assembler.or_int_16(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 4)
			{
				assembler.or_int_32(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 8)
			{
				assembler.or_int_64(rhs_reg, result_reg);
			}

			break;
		}

		case LEFT_SHIFT:
		{
			if (type.is_integer() && type.byte_size() == 1)
			{
				assembler.shl_int_8(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 2)
			{
				assembler.shl_int_16(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 4)
			{
				assembler.shl_int_32(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 8)
			{
				assembler.shl_int_64(rhs_reg, result_reg);
			}

			break;
		}

		case RIGHT_SHIFT:
		{
			if (type.is_integer() && type.byte_size() == 1)
			{
				assembler.shr_int_8(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 2)
			{
				assembler.shr_int_16(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 4)
			{
				assembler.shr_int_32(rhs_reg, result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 8)
			{
				assembler.shr_int_64(rhs_reg, result_reg);
			}

			break;
		}

			// Logical binary operations

		case LESS:
			compare_and_set([&]()
				{ assembler.set_if_lt(result_reg); });
			break;

		case LESS_OR_EQUAL:
			compare_and_set([&]()
				{ assembler.set_if_leq(result_reg); });
			break;

		case GREATER:
			compare_and_set([&]()
				{ assembler.set_if_gt(result_reg); });
			break;

		case GREATER_OR_EQUAL:
			compare_and_set([&]()
				{ assembler.set_if_geq(result_reg); });
			break;

		case EQUAL:
			compare_and_set([&]()
				{ assembler.set_if_eq(result_reg); });
			break;

		case NOT_EQUAL:
			compare_and_set([&]()
				{ assembler.set_if_neq(result_reg); });
			break;

		default:
			p_warn(stderr, "operator %d in BinaryOperation not implemented\n",
				op);
			abort();
			break;
		}

#undef COMPARE_AND_SET

		assembler.free_register(rhs_reg);
	}
};

constexpr int BINARY_OPERATION_SIZE = sizeof(BinaryOperation);

#endif