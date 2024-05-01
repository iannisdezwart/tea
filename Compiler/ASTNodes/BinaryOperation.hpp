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

		if (left->type.is_integer())
		{
			warn("binary operation on non-integer type x = %s\n"
			     "At %ld:%ld\n",
				left->type.to_str().c_str(), accountable_token.line,
				accountable_token.col);
		}
		if (right->type.is_integer())
		{
			warn("binary operation on non-integer type y = %s\n"
			     "At %ld:%ld\n",
				right->type.to_str().c_str(), accountable_token.line,
				accountable_token.col);
		}

		size_t left_size  = left->type.byte_size();
		size_t right_size = right->type.byte_size();

		switch (op)
		{
		case ADDITION:
		case SUBTRACTION:
		{
			// intX + intY -> max(intX, intY)

			if (left->type.pointer_depth() == 0
				&& right->type.pointer_depth() == 0)
			{
				if (left_size > right_size)
				{
					type = left->type;
					return;
				}

				type = right->type;
				return;
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
		{
			// intX * intY -> max(intX, intY)

			if (left->type.pointer_depth() == 0
				&& right->type.pointer_depth() == 0)
			{
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

		case DIVISION:
		{
			// intX / intY -> intX

			if (left->type.pointer_depth() == 0
				&& right->type.pointer_depth() == 0)
			{
				type = left->type;
				return;
			}

			break;
		}

		case REMAINDER:
		{
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
		{
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
				warn("comparing types of different signedness x = %s and y = %s\n"
				     "At %ld:%ld\n",
					left->type.to_str().c_str(),
					right->type.to_str().c_str(), accountable_token.line,
					accountable_token.col);
			}

			type = Type(left->type, 1);
			return;
		}

		default:
			printf("operator %s in BinaryOperation not implemented\n",
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

		// Perform the operation.

		switch (op)
		{
			// Mathematical binary operations.

		case DIVISION:
			assembler.divide_reg_from_reg(rhs_reg, result_reg);
			break;

		case REMAINDER:
			assembler.take_modulo_reg_of_reg(rhs_reg, result_reg);
			break;

		case MULTIPLICATION:
			assembler.multiply_reg_into_reg(rhs_reg, result_reg);
			break;

		case ADDITION:
			assembler.add_reg_into_reg(rhs_reg, result_reg);
			break;

		case SUBTRACTION:
			assembler.subtract_reg_from_reg(rhs_reg, result_reg);
			break;

		case BITWISE_AND:
			assembler.and_reg_into_reg(rhs_reg, result_reg);
			break;

		case BITWISE_XOR:
			assembler.xor_reg_into_reg(rhs_reg, result_reg);
			break;

		case BITWISE_OR:
			assembler.or_reg_into_reg(rhs_reg, result_reg);
			break;

			// Logical binary operations

#define COMPARE_AND_SET(op)                                                         \
	do                                                                          \
	{                                                                           \
		if (type == Type::SIGNED_INTEGER)                                   \
			assembler.compare_reg_to_reg_signed(result_reg, rhs_reg);   \
		else                                                                \
			assembler.compare_reg_to_reg_unsigned(result_reg, rhs_reg); \
		assembler.set_reg_if_##op(result_reg);                              \
	} while (0)

		case LESS:
			COMPARE_AND_SET(less);
			break;

		case LESS_OR_EQUAL:
			COMPARE_AND_SET(less_or_equal);
			break;

		case GREATER:
			COMPARE_AND_SET(greater);
			break;

		case GREATER_OR_EQUAL:
			COMPARE_AND_SET(greater_or_equal);
			break;

		case EQUAL:
			COMPARE_AND_SET(equal);
			break;

		case NOT_EQUAL:
			COMPARE_AND_SET(not_equal);
			break;

		default:
			printf("operator %d in BinaryOperation not implemented\n",
				op);
			abort();
			break;
		}

#undef COMPARE_AND_SET

		assembler.free_register(rhs_reg);
	}
};

#endif