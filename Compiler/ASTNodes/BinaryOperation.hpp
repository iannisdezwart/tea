#ifndef TEA_AST_NODE_BINARY_OPERATION_HEADER
#define TEA_AST_NODE_BINARY_OPERATION_HEADER

#include "../util.hpp"
#include "ASTNode.hpp"
#include "ReadValue.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../../Assembler/assembler.hpp"
#include "../compiler-state.hpp"
#include "../tokeniser.hpp"
#include "../../VM/cpu.hpp"

struct BinaryOperation : public ReadValue
{
	ReadValue *left;
	ReadValue *right;
	Token op_token;
	enum Operator op;

	bool warned = false;

	BinaryOperation(ReadValue *left, ReadValue *right, Token op_token)
		: left(left), right(right), op_token(op_token),
		  op(str_to_operator(op_token.value)),
		  ReadValue(op_token, BINARY_OPERATION) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
	{
		left->dfs(callback, depth + 1);
		right->dfs(callback, depth + 1);

		callback(this, depth);
	}

	std::string
	to_str()
	{
		std::string s;

		s += "BinaryOperation { op = \"";
		s += op_to_str(op);
		s += "\" } @ ";
		s += to_hex((size_t) this);

		return s;
	}

	Type
	get_type(CompilerState &compiler_state)
	{
		Type left_type  = left->get_type(compiler_state);
		Type right_type = right->get_type(compiler_state);

		size_t left_size  = left_type.byte_size();
		size_t right_size = right_type.byte_size();

		switch (op)
		{
		case ADDITION:
		case SUBTRACTION:
		{
			// intX + intY -> max(intX, intY)

			if (left_type.pointer_depth() == 0
				&& right_type.pointer_depth() == 0)
			{
				if (left_size > right_size)
				{
					return left_type;
				}

				return right_type;
			}

			// pointer + int -> pointer

			if (left_type.pointer_depth() > 0
				&& right_type.pointer_depth() == 0)
			{
				return left_type;
			}

			// int + pointer -> pointer

			if (right_type.pointer_depth() > 0
				&& left_type.pointer_depth() == 0)
			{
				return right_type;
			}

			break;
		}

		case MULTIPLICATION:
		{
			// intX * intY -> max(intX, intY)

			if (left_type.pointer_depth() == 0
				&& right_type.pointer_depth() == 0)
			{
				if (left_size > right_size)
				{
					return left_type;
				}

				return right_type;
			}

			break;
		}

		case DIVISION:
		{
			// intX / intY -> intX

			if (left_type.pointer_depth() == 0
				&& right_type.pointer_depth() == 0)
			{
				return left_type;
			}

			break;
		}

		case REMAINDER:
		{
			// intX % intY -> intX

			if (left_type.pointer_depth() == 0
				&& right_type.pointer_depth() == 0)
			{
				return left_type;
			}
		}

		case BITWISE_AND:
		case BITWISE_XOR:
		case BITWISE_OR:
		{
			// intX & intX

			if (left_type.pointer_depth() == 0
				&& right_type.pointer_depth() == 0
				&& left_size == right_size)
			{
				return left_type;
			}
		}

		case LESS:
		case LESS_OR_EQUAL:
		case GREATER:
		case GREATER_OR_EQUAL:
		case EQUAL:
		case NOT_EQUAL:
		{
			if (left_type != Type::SIGNED_INTEGER && left_type != Type::UNSIGNED_INTEGER)
			{
				warn("comparing non-integer type x = %s\n"
				     "At %ld:%ld\n",
					left_type.to_str().c_str(), op_token.line,
					op_token.col);
			}
			if (right_type != Type::SIGNED_INTEGER && right_type != Type::UNSIGNED_INTEGER)
			{
				warn("comparing non-integer type y = %s\n"
				     "At %ld:%ld\n",
					right_type.to_str().c_str(), op_token.line,
					op_token.col);
			}
			if (left_type != right_type)
			{
				warn("comparing types of different signedness x = %s and y = %s\n"
				     "At %ld:%ld\n",
					left_type.to_str().c_str(),
					right_type.to_str().c_str(), op_token.line,
					op_token.col);
			}

			return Type(left_type, 1);
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

		if (!warned)
		{
			warned = true;
			warn("operator %s (%s) is not implemented for types x = %s and y = %s "
			     "and might cause undefined behaviour\n"
			     "At %ld:%ld\n",
				op_to_str(op), op_to_example_str(op),
				left_type.to_str().c_str(),
				right_type.to_str().c_str(), op_token.line,
				op_token.col);
		}

		return left_type;
	}

	void
	get_value(Assembler &assembler, CompilerState &compiler_state,
		uint8_t result_reg)
	{
		uint8_t rhs_reg;

		Type type = get_type(compiler_state);

		// Get the left hand side value.

		left->get_value(assembler, compiler_state, result_reg);

		// Get the right hand side value.

		rhs_reg = assembler.get_register();
		right->get_value(assembler, compiler_state, rhs_reg);

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