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
#include "VM/cpu.hpp"

struct UnaryOperation final : public WriteValue
{
	Operator op;
	std::unique_ptr<ReadValue> expression;
	bool prefix;

	bool warned = false;

	UnaryOperation(CompactToken accountable_token,
		Operator op, bool prefix,
		std::unique_ptr<ReadValue> expression)
		: WriteValue(std::move(accountable_token), UNARY_OPERATION),
		  op(op), prefix(prefix),
		  expression(std::move(expression)) {}

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
		type = expression->type;

		switch (op)
		{
		case POSTFIX_INCREMENT:
		case POSTFIX_DECREMENT:
		case PREFIX_INCREMENT:
		case PREFIX_DECREMENT:
		case UNARY_PLUS:
		case UNARY_MINUS:
		case BITWISE_NOT:
		{
			if (!type.is_primitive())
			{
				err_at_token(accountable_token, "Internal Error",
					"Unsupported type (%s) for operator %s.\n"
					"Expected a primitive type",
					type.to_str().c_str(), op_to_str(op));
			}
			break;
		}
		case LOGICAL_NOT:
		{
			if (!type.is_primitive() && type.pointer_depth() == 0)
			{
				err_at_token(accountable_token, "Internal Error",
					"Unsupported type (%s) for operator %s.\n"
					"Expected a primitive type or pointer",
					type.to_str().c_str(), op_to_str(op));
			}
			break;
		}

		case DEREFERENCE:
		{
			if (type.pointer_depth() == 0)
			{
				err_at_token(accountable_token,
					"Cannot dereference a non-pointer",
					"type %s cannot be dereferenced",
					type.to_str().c_str());
			}

			type.array_sizes.pop_back();
			break;
		}

		case ADDRESS_OF:
		{
			type.array_sizes.insert(type.array_sizes.begin(), 1, 0);
			break;
		}

		default:
		{
			err_at_token(accountable_token, "Invalid UnaryOperation",
				"didn't find an operation to perform with operator %s",
				op_to_str(op));
		}
		}
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

			while (--deref_dep > 0)
			{
				expr_type.array_sizes.pop_back();
				assembler.load_ptr_64(ptr_reg, ptr_reg);
			}

			switch (expr->type.byte_size())
			{
			case 1:
				assembler.store_ptr_8(value_reg, ptr_reg);
				break;

			case 2:
				assembler.store_ptr_16(value_reg, ptr_reg);
				break;

			case 4:
				assembler.store_ptr_32(value_reg, ptr_reg);
				break;

			case 8:
				assembler.store_ptr_64(value_reg, ptr_reg);
				break;

			default:
				// TODO: Test if this works.
				assembler.mem_copy(value_reg, ptr_reg, expr->type.byte_size());
				break;
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
	increment(Assembler &assembler, const Type &type, uint8_t result_reg)
		const
	{
		if (type.pointer_depth() > 0)
		{
			uint8_t temp_reg = assembler.get_register();
			assembler.move_lit(type.pointed_byte_size(), temp_reg);
			assembler.add_int_64(temp_reg, result_reg);
			assembler.free_register(temp_reg);
		}
		else if (type.is_integer() && type.byte_size() == 1)
		{
			assembler.inc_int_8(result_reg);
		}
		else if (type.is_integer() && type.byte_size() == 2)
		{
			assembler.inc_int_16(result_reg);
		}
		else if (type.is_integer() && type.byte_size() == 4)
		{
			assembler.inc_int_32(result_reg);
		}
		else if (type.is_integer() && type.byte_size() == 8)
		{
			assembler.inc_int_64(result_reg);
		}
		else if (type.value == F32)
		{
			uint8_t temp_reg = assembler.get_register();
			float one        = 1.0f;
			assembler.move_lit(*reinterpret_cast<uint32_t *>(&one), temp_reg);
			assembler.add_flt_32(temp_reg, result_reg);
			assembler.free_register(temp_reg);
		}
		else if (type.value == F64)
		{
			uint8_t temp_reg = assembler.get_register();
			double one       = 1.0;
			assembler.move_lit(*reinterpret_cast<uint64_t *>(&one), temp_reg);
			assembler.add_flt_64(temp_reg, result_reg);
			assembler.free_register(temp_reg);
		}
	}

	void
	decrement(Assembler &assembler, const Type &type, uint8_t result_reg)
		const
	{
		if (type.pointer_depth() > 0)
		{
			uint8_t temp_reg = assembler.get_register();
			assembler.move_lit(type.pointed_byte_size(), temp_reg);
			assembler.sub_int_64(temp_reg, result_reg);
			assembler.free_register(temp_reg);
		}
		else if (type.is_integer() && type.byte_size() == 1)
		{
			assembler.dec_int_8(result_reg);
		}
		else if (type.is_integer() && type.byte_size() == 2)
		{
			assembler.dec_int_16(result_reg);
		}
		else if (type.is_integer() && type.byte_size() == 4)
		{
			assembler.dec_int_32(result_reg);
		}
		else if (type.is_integer() && type.byte_size() == 8)
		{
			assembler.dec_int_64(result_reg);
		}
		else if (type.value == F32)
		{
			uint8_t temp_reg = assembler.get_register();
			float one        = 1.0f;
			assembler.move_lit(*reinterpret_cast<uint32_t *>(&one), temp_reg);
			assembler.sub_flt_32(temp_reg, result_reg);
			assembler.free_register(temp_reg);
		}
		else if (type.value == F64)
		{
			uint8_t temp_reg = assembler.get_register();
			double one       = 1.0;
			assembler.move_lit(*reinterpret_cast<uint64_t *>(&one), temp_reg);
			assembler.sub_flt_64(temp_reg, result_reg);
			assembler.free_register(temp_reg);
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

			wr_expression->get_value(assembler, result_reg);
			increment(assembler, type, result_reg);
			wr_expression->store(assembler, result_reg);
			decrement(assembler, type, result_reg);

			break;
		}

		case POSTFIX_DECREMENT:
		{
			WriteValue *wr_expression = WriteValue::cast(expression.get());
			Type type                 = wr_expression->type;

			wr_expression->get_value(assembler, result_reg);
			decrement(assembler, type, result_reg);
			wr_expression->store(assembler, result_reg);
			increment(assembler, type, result_reg);

			break;
		}

		case PREFIX_INCREMENT:
		{
			WriteValue *wr_expression = WriteValue::cast(expression.get());
			Type type                 = wr_expression->type;

			wr_expression->get_value(assembler, result_reg);
			increment(assembler, type, result_reg);
			wr_expression->store(assembler, result_reg);

			break;
		}

		case PREFIX_DECREMENT:
		{
			WriteValue *wr_expression = WriteValue::cast(expression.get());
			Type type                 = wr_expression->type;

			wr_expression->get_value(assembler, result_reg);
			decrement(assembler, type, result_reg);
			wr_expression->store(assembler, result_reg);

			break;
		}

		case UNARY_PLUS:
		{
			expression->get_value(assembler, result_reg);
			break;
		}

		case UNARY_MINUS:
		{
			expression->get_value(assembler, result_reg);

			// Two's complement for ints and XOR for floats.

			if (type.is_integer() && type.byte_size() == 1)
			{
				assembler.neg_int_8(result_reg);
				assembler.inc_int_8(result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 2)
			{
				assembler.neg_int_16(result_reg);
				assembler.inc_int_16(result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 4)
			{
				assembler.neg_int_32(result_reg);
				assembler.inc_int_32(result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 8)
			{
				assembler.neg_int_64(result_reg);
				assembler.inc_int_64(result_reg);
			}
			else if (type.value == F32)
			{
				uint8_t temp_reg  = assembler.get_register();
				uint32_t sign_bit = 0x80000000;
				assembler.move_lit(sign_bit, temp_reg);
				assembler.xor_int_32(temp_reg, result_reg);
				assembler.free_register(temp_reg);
			}
			else if (type.value == F64)
			{
				uint8_t temp_reg  = assembler.get_register();
				uint64_t sign_bit = 0x8000000000000000;
				assembler.move_lit(sign_bit, temp_reg);
				assembler.xor_int_64(temp_reg, result_reg);
				assembler.free_register(temp_reg);
			}

			break;
		}

		case BITWISE_NOT:
		case LOGICAL_NOT:
		{
			expression->get_value(assembler, result_reg);

			if (type.pointer_depth() > 0)
			{
				assembler.neg_int_64(result_reg);
			}
			if (type.is_integer() && type.byte_size() == 1)
			{
				assembler.neg_int_8(result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 2)
			{
				assembler.neg_int_16(result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 4)
			{
				assembler.neg_int_32(result_reg);
			}
			else if (type.is_integer() && type.byte_size() == 8)
			{
				assembler.neg_int_64(result_reg);
			}

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
				assembler.load_ptr_8(result_reg, result_reg);
				break;

			case 2:
				assembler.load_ptr_16(result_reg, result_reg);
				break;

			case 4:
				assembler.load_ptr_32(result_reg, result_reg);
				break;

			case 8:
				assembler.load_ptr_64(result_reg, result_reg);
				break;

			default:
				break;
			}

			break;
		}

		case ADDRESS_OF:
		{
			WriteValue *wr_expression = WriteValue::cast(expression.get());

			if (wr_expression->location_data.is_at_frame_top())
			{
				assembler.move_lit(wr_expression->location_data.offset, result_reg);
				assembler.add_int_64(R_FRAME_PTR, result_reg);
			}

			else
			{
				assembler.move_lit(wr_expression->location_data.offset, result_reg);
				assembler.add_int_64(R_STACK_TOP_PTR, result_reg);
			}

			break;
		}

		default:
		{
			err_at_token(accountable_token, "Invalid UnaryOperation",
				"didn't find an operation to perform with operator %s",
				op_to_str(op));
		}
		}
	}
};

constexpr int UNARY_OPERATION_SIZE = sizeof(UnaryOperation);

#endif