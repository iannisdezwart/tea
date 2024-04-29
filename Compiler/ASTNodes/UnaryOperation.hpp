#ifndef TEA_AST_NODES_UNARY_OPERATION_HEADER
#define TEA_AST_NODES_UNARY_OPERATION_HEADER

#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/ASTNodes/ReadValue.hpp"
#include "Compiler/ASTNodes/WriteValue.hpp"
#include "Compiler/ASTNodes/IdentifierExpression.hpp"
#include "Compiler/ASTNodes/AssignmentExpression.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/code-gen/Assembler.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"
#include "Compiler/tokeniser.hpp"
#include "VM/cpu.hpp"

struct UnaryOperation final : public WriteValue
{
	std::unique_ptr<ReadValue> expression;
	Operator op;
	bool prefix;

	bool warned = false;

	UnaryOperation(std::unique_ptr<ReadValue> expression, Token op_token, bool prefix)
		: WriteValue(std::move(op_token), UNARY_OPERATION),
		  expression(std::move(expression)),
		  op(str_to_operator(accountable_token.value, prefix)),
		  prefix(prefix) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
		override
	{
		expression->dfs(callback, depth + 1);
		callback(this, depth);
	}

	std::string
	to_str()
		override
	{
		std::string s;

		s += "UnaryOperation { op = \"";
		s += op_to_str(op);
		s += "\" } @ ";
		s += to_hex((size_t) this);

		return s;
	}

	void
	type_check(TypeCheckState &type_check_state)
		override
	{
		expression->type_check(type_check_state);

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
			type = expression->type;
			return;
		}

		case DEREFERENCE:
		{
			type = expression->type;

			if (type.pointer_depth() == 0)
			{
				err_at_token(accountable_token,
					"Cannot dereference a non-pointer",
					"type %s cannot be dereferenced",
					type.to_str().c_str());
			}

			type.array_sizes.pop_back();
			return;
		}

		case ADDRESS_OF:
		{
			type = expression->type;
			type.array_sizes.insert(type.array_sizes.begin(), 1, 0);
			return;
		}

		default:
		{
			err_at_token(accountable_token, "Invalid UnaryOperation",
				"didn't find an operation to perform with operator %s",
				accountable_token.value.c_str());
		}
		}

		// TODO: set location data.
	}

	void
	store(Assembler &assembler, uint8_t value_reg)
		const override
	{
		switch (op)
		{
		case PREFIX_INCREMENT:
		{
			WriteValue *wr_expression = WriteValue::cast(expression.get());
			wr_expression->store(assembler, value_reg);
			break;
		}

		case PREFIX_DECREMENT:
		{
			WriteValue *wr_expression = WriteValue::cast(expression.get());
			wr_expression->store(assembler, value_reg);
			break;
		}

		case DEREFERENCE:
		{
			uint8_t ptr_reg = assembler.get_register();

			size_t deref_dep      = 0;
			const ReadValue *expr = this;

			while (
				expr->node_type == UNARY_OPERATION
				&& ((UnaryOperation *) expr)->op == DEREFERENCE)
			{
				expr = ((UnaryOperation *) expr)->expression.get();
				deref_dep++;
			}

			// Move the address of what to dereference into the pointer register.

			expr->get_value(assembler, ptr_reg);

			// Dereference.

			Type expr_type = expr->type;

			while (--deref_dep)
			{
				expr_type.array_sizes.pop_back();
				assembler.move_reg_pointer_64_into_reg(
					ptr_reg, ptr_reg);
			}

			switch (expr->type.byte_size())
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
					expr->type.byte_size());
				abort();
			}

			assembler.free_register(ptr_reg);

			break;
		}

		default:
			err_at_token(accountable_token,
				"Value Type Error",
				"Expected a WriteValue\n"
				"This value is not writable");
		}
	}

	void
	get_value(Assembler &assembler, uint8_t result_reg)
		const override
	{
		switch (op)
		{
		case POSTFIX_INCREMENT:
		{
			WriteValue *wr_expression = WriteValue::cast(expression.get());
			Type type                 = wr_expression->type;

			// Move result into the result reg and increment it.

			wr_expression->get_value(assembler, result_reg);

			if (type.pointer_depth() > 0)
			{
				assembler.add_64_into_reg(type.pointed_byte_size(), result_reg);
			}
			else
			{
				assembler.increment_reg(result_reg);
			}

			// Store the value back into memory.

			wr_expression->store(assembler, result_reg);

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
			WriteValue *wr_expression = WriteValue::cast(expression.get());
			Type type                 = wr_expression->type;

			// Move result into the result reg and decrement it.

			wr_expression->get_value(assembler, result_reg);

			if (type.pointer_depth() > 0)
			{
				assembler.subtract_64_from_reg(type.pointed_byte_size(), result_reg);
			}
			else
			{
				assembler.decrement_reg(result_reg);
			}

			// Store the value back into memory.

			wr_expression->store(assembler, result_reg);

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
			WriteValue *wr_expression = WriteValue::cast(expression.get());
			Type type                 = wr_expression->type;

			// Move result into the result reg and increment it.

			wr_expression->get_value(assembler, result_reg);

			if (type.pointer_depth() > 0)
			{
				assembler.add_64_into_reg(type.pointed_byte_size(), result_reg);
			}
			else
			{
				assembler.increment_reg(result_reg);
			}

			// Store the value back into memory.

			wr_expression->store(assembler, result_reg);
			break;
		}

		case PREFIX_DECREMENT:
		{
			WriteValue *wr_expression = WriteValue::cast(expression.get());
			Type type                 = wr_expression->type;

			// Move result into the result reg and decrement it.

			wr_expression->get_value(assembler, result_reg);

			if (type.pointer_depth() > 0)
			{
				assembler.subtract_64_from_reg(type.pointed_byte_size(), result_reg);
			}
			else
			{
				assembler.decrement_reg(result_reg);
			}

			// Store the value back into memory.

			wr_expression->store(assembler, result_reg);
			break;
		}

		case UNARY_PLUS:
		{
			// Todo: look up if this operator does anything interesting.

			expression->get_value(assembler, result_reg);
			break;
		}

		case UNARY_MINUS:
		{
			expression->get_value(assembler, result_reg);

			// Invert the bits and add one.

			assembler.not_reg(result_reg);
			assembler.increment_reg(result_reg);
			break;
		}

		case BITWISE_NOT:
		{
			expression->get_value(assembler, result_reg);

			// Invert the bits.

			assembler.not_reg(result_reg);
			break;
		}

		case LOGICAL_NOT:
		{
			expression->get_value(assembler, result_reg);

			// Invert the bits and mask.

			assembler.not_reg(result_reg);
			assembler.and_64_into_reg(1, result_reg);
			break;
		}

		case DEREFERENCE:
		{
			// Moves the address of what to dereference into the result reg.

			expression->get_value(assembler, result_reg);
			Type type = expression->type;
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
			WriteValue *wr_expression = WriteValue::cast(expression.get());

			if (wr_expression->location_data->is_at_frame_top())
			{
				assembler.move_reg_into_reg(R_FRAME_PTR, result_reg);
				assembler.add_64_into_reg(
					wr_expression->location_data->offset, result_reg);
			}

			else
			{
				assembler.move_stack_top_address_into_reg(result_reg);
				assembler.add_64_into_reg(
					wr_expression->location_data->offset, result_reg);
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