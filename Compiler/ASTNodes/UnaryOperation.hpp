#ifndef TEA_AST_NODES_UNARY_OPERATION_HEADER
#define TEA_AST_NODES_UNARY_OPERATION_HEADER

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

struct UnaryOperation : public WriteValue
{
	ReadValue *expression;
	Token op_token;
	bool prefix;
	enum Operator op;

	bool warned = false;

	UnaryOperation(ReadValue *expression, const Token &op_token, bool prefix)
		: expression(expression), op_token(op_token),
		  op(str_to_operator(op_token.value, prefix)),
		  prefix(prefix), WriteValue(op_token, UNARY_OPERATION) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
	{
		expression->dfs(callback, depth + 1);
		callback(this, depth);
	}

	std::string
	to_str()
	{
		std::string s;

		s += "UnaryOperation { op = \"";
		s += op_to_str(op);
		s += "\" } @ ";
		s += to_hex((size_t) this);

		return s;
	}

	Type
	get_type(CompilerState &compiler_state)
	{
		Type type = expression->get_type(compiler_state);

		switch (op)
		{
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
			if (type.pointer_depth() == 0)
			{
				err_at_token(op_token,
					"Cannot dereference a non-pointer",
					"type %s cannot be dereferenced",
					type.to_str().c_str());
			}

			type.array_sizes.pop_back();
			return type;
		}

		case ADDRESS_OF:
		{
			type.array_sizes.insert(type.array_sizes.begin(), 1, 0);
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

	LocationData
	get_location_data(CompilerState &compiler_state)
	{
		// Todo: create.
	}

	void
	store(Assembler &assembler, CompilerState &compiler_state,
		uint8_t value_reg)
	{
		switch (op)
		{
		case PREFIX_INCREMENT:
		{
			WriteValue *wr_expression = WriteValue::cast(expression);
			wr_expression->store(assembler, compiler_state, value_reg);
			break;
		}

		case PREFIX_DECREMENT:
		{
			WriteValue *wr_expression = WriteValue::cast(expression);
			wr_expression->store(assembler, compiler_state, value_reg);
			break;
		}

		case DEREFERENCE:
		{
			uint8_t ptr_reg = assembler.get_register();

			size_t deref_dep = 0;
			ReadValue *expr  = this;

			while (
				expr->type == UNARY_OPERATION
				&& ((UnaryOperation *) expr)->op == DEREFERENCE)
			{
				expr = ((UnaryOperation *) expr)->expression;
				deref_dep++;
			}

			// Move the address of what to dereference into the pointer register.

			expr->get_value(assembler, compiler_state, ptr_reg);

			Type expr_type = expr->get_type(compiler_state);

			// Dereference.

			while (--deref_dep)
			{
				expr_type.array_sizes.pop_back();
				assembler.move_reg_pointer_64_into_reg(
					ptr_reg, ptr_reg);
			}

			switch (expr_type.byte_size())
			{
			case 1:
				assembler.move_reg_into_reg_pointer_8(
					value_reg, ptr_reg);
				break;

			case 2:
				assembler.move_reg_into_reg_pointer_16(
					value_reg, ptr_reg);
				break;

			case 4:
				assembler.move_reg_into_reg_pointer_32(
					value_reg, ptr_reg);
				break;

			case 8:
				assembler.move_reg_into_reg_pointer_64(
					value_reg, ptr_reg);
				break;

			default:
				printf("Dereference assignment for "
				       "	byte size %lu is not implemented\n",
					expr->get_type(compiler_state).byte_size());
				abort();
			}

			assembler.free_register(ptr_reg);

			break;
		}

		default:
			err_at_token(op_token,
				"Value Type Error",
				"Expected a WriteValue\n"
				"This value is not writable");
		}
	}

	void
	get_value(Assembler &assembler, CompilerState &compiler_state, uint8_t result_reg)
	{
		switch (op)
		{
		case POSTFIX_INCREMENT:
		{
			WriteValue *wr_expression = WriteValue::cast(expression);
			Type type                 = wr_expression->get_type(compiler_state);

			// Move result into the result reg and increment it.

			wr_expression->get_value(assembler, compiler_state, result_reg);

			if (type.pointer_depth() > 0)
			{
				assembler.add_64_into_reg(type.pointed_byte_size(), result_reg);
			}
			else
			{
				assembler.increment_reg(result_reg);
			}

			// Store the value back into memory.

			wr_expression->store(assembler, compiler_state, result_reg);

			// Decrement the result reg.

			if (type.pointer_depth() > 0)
			{
				assembler.subtract_64_from_reg(type.pointed_byte_size(), result_reg);
			}
			else
			{
				assembler.decrement_reg(result_reg);
			}

			break;
		}

		case POSTFIX_DECREMENT:
		{
			WriteValue *wr_expression = WriteValue::cast(expression);
			Type type                 = wr_expression->get_type(compiler_state);

			// Move result into the result reg and decrement it.

			wr_expression->get_value(assembler, compiler_state, result_reg);

			if (type.pointer_depth() > 0)
			{
				assembler.subtract_64_from_reg(type.pointed_byte_size(), result_reg);
			}
			else
			{
				assembler.decrement_reg(result_reg);
			}

			// Store the value back into memory.

			wr_expression->store(assembler, compiler_state, result_reg);

			// Increment the result reg.

			if (type.pointer_depth() > 0)
			{
				assembler.add_64_into_reg(type.pointed_byte_size(), result_reg);
			}
			else
			{
				assembler.increment_reg(result_reg);
			}

			break;
		}

		case PREFIX_INCREMENT:
		{
			WriteValue *wr_expression = WriteValue::cast(expression);
			Type type                 = wr_expression->get_type(compiler_state);

			// Move result into the result reg and increment it.

			wr_expression->get_value(assembler, compiler_state, result_reg);

			if (type.pointer_depth() > 0)
			{
				assembler.add_64_into_reg(type.pointed_byte_size(), result_reg);
			}
			else
			{
				assembler.increment_reg(result_reg);
			}

			// Store the value back into memory.

			wr_expression->store(assembler, compiler_state, result_reg);
			break;
		}

		case PREFIX_DECREMENT:
		{
			WriteValue *wr_expression = WriteValue::cast(expression);
			Type type                 = wr_expression->get_type(compiler_state);

			// Move result into the result reg and decrement it.

			wr_expression->get_value(assembler, compiler_state, result_reg);

			if (type.pointer_depth() > 0)
			{
				assembler.subtract_64_from_reg(type.pointed_byte_size(), result_reg);
			}
			else
			{
				assembler.decrement_reg(result_reg);
			}

			// Store the value back into memory.

			wr_expression->store(assembler, compiler_state, result_reg);
			break;
		}

		case UNARY_PLUS:
		{
			// Todo: look up if this operator does anything interesting.

			expression->get_value(assembler, compiler_state, result_reg);
			break;
		}

		case UNARY_MINUS:
		{
			expression->get_value(assembler, compiler_state, result_reg);

			// Invert the bits and add one.

			assembler.not_reg(result_reg);
			assembler.increment_reg(result_reg);
			break;
		}

		case BITWISE_NOT:
		{
			expression->get_value(assembler, compiler_state, result_reg);

			// Invert the bits.

			assembler.not_reg(result_reg);
			break;
		}

		case LOGICAL_NOT:
		{
			expression->get_value(assembler, compiler_state, result_reg);

			// Invert the bits and mask.

			assembler.not_reg(result_reg);
			assembler.and_64_into_reg(1, result_reg);
			break;
		}

		case DEREFERENCE:
		{
			// Moves the address of what to dereference into the result reg.

			expression->get_value(assembler, compiler_state, result_reg);
			Type type = expression->get_type(compiler_state);
			type.array_sizes.pop_back();

			// Move the dereferenced value into the result reg.

			switch (type.byte_size())
			{
			case 1:
				assembler.move_reg_pointer_8_into_reg(result_reg, result_reg);
				break;

			case 2:
				assembler.move_reg_pointer_16_into_reg(result_reg, result_reg);
				break;

			case 4:
				assembler.move_reg_pointer_32_into_reg(result_reg, result_reg);
				break;

			case 8:
				assembler.move_reg_pointer_64_into_reg(result_reg, result_reg);
				break;
			}

			break;
		}

		case ADDRESS_OF:
		{
			WriteValue *wr_expression  = WriteValue::cast(expression);
			LocationData location_data = wr_expression->get_location_data(compiler_state);

			if (location_data.is_at_frame_top())
			{
				assembler.move_reg_into_reg(R_FRAME_PTR, result_reg);
				assembler.add_64_into_reg(location_data.offset, result_reg);
			}

			else
			{
				assembler.move_stack_top_address_into_reg(result_reg);
				assembler.add_64_into_reg(location_data.offset, result_reg);
			}

			break;
		}

		default:
		{
			err_at_token(expression->accountable_token, "Internal Error",
				"Unsupported unary operator");
		}
		}
	}
};

#endif