#ifndef TEA_AST_NODE_OFFSET_EXPRESSION_HEADER
#define TEA_AST_NODE_OFFSET_EXPRESSION_HEADER

#include "Compiler/ASTNodes/ReadValue.hpp"
#include "Compiler/ASTNodes/WriteValue.hpp"

struct OffsetExpression final : public WriteValue
{
	std::unique_ptr<WriteValue> pointer;
	std::unique_ptr<ReadValue> offset;

	OffsetExpression(std::unique_ptr<WriteValue> pointer,
		std::unique_ptr<ReadValue> offset, Token bracket_token)
		: WriteValue(std::move(bracket_token), OFFSET_EXPRESSION),
		  pointer(std::move(pointer)),
		  offset(std::move(offset)) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
		override
	{
		pointer->dfs(callback, depth + 1);
		offset->dfs(callback, depth + 1);

		callback(this, depth);
	}

	std::string
	to_str()
		override
	{
		std::string s;

		s += "OffsetExpression {} @ ";
		s += to_hex((size_t) this);

		return s;
	}

	void
	type_check(TypeCheckState &type_check_state)
		override
	{
		pointer->type_check(type_check_state);
		offset->type_check(type_check_state);

		if (offset->type.pointer_depth())
		{
			err_at_token(accountable_token,
				"Type Error",
				"Offset provided in OffsetExpression is not an integer\n"
				"Found type %s instead",
				offset->type.to_str().c_str());
		}

		type          = pointer->type.pointed_type();
		location_data = LocationData(pointer->location_data);
	}

	void
	store(Assembler &assembler, uint8_t value_reg)
		const override
	{
		Type pointed_type  = type;
		uint8_t offset_reg = assembler.get_register();

		// Local variable or parameter

		if (location_data.is_at_frame_top())
		{
			switch (type.byte_size())
			{
			case 1:
				offset->get_value(assembler, offset_reg);
				assembler.multiply_8_into_reg(pointed_type.byte_size(), offset_reg);
				assembler.add_reg_into_reg(R_FRAME_PTR, offset_reg);
				assembler.add_64_into_reg(location_data.offset, offset_reg);
				assembler.move_reg_into_reg_pointer_8(value_reg, offset_reg);
				break;

			case 2:
				offset->get_value(assembler, offset_reg);
				assembler.multiply_8_into_reg(pointed_type.byte_size(), offset_reg);
				assembler.add_reg_into_reg(R_FRAME_PTR, offset_reg);
				assembler.add_64_into_reg(location_data.offset, offset_reg);
				assembler.move_reg_into_reg_pointer_16(value_reg, offset_reg);
				break;

			case 4:
				offset->get_value(assembler, offset_reg);
				assembler.multiply_8_into_reg(pointed_type.byte_size(), offset_reg);
				assembler.add_reg_into_reg(R_FRAME_PTR, offset_reg);
				assembler.add_64_into_reg(location_data.offset, offset_reg);
				assembler.move_reg_into_reg_pointer_32(value_reg, offset_reg);
				break;

			case 8:
				offset->get_value(assembler, offset_reg);
				assembler.multiply_8_into_reg(pointed_type.byte_size(), offset_reg);
				assembler.add_reg_into_reg(R_FRAME_PTR, offset_reg);
				assembler.add_64_into_reg(location_data.offset, offset_reg);
				assembler.move_reg_into_reg_pointer_64(value_reg, offset_reg);
				break;

			default:
				err_at_token(pointer->accountable_token,
					"Type Error",
					"Variable doesn't fit in register\n"
					"Support for this is not implemented yet");
			}

			assembler.free_register(offset_reg);
			return;
		}

		// Global variable

		switch (type.byte_size())
		{
		case 1:
			offset->get_value(assembler, offset_reg);
			assembler.multiply_8_into_reg(pointed_type.byte_size(), offset_reg);
			assembler.move_stack_top_address_into_reg(offset_reg);
			assembler.add_64_into_reg(location_data.offset, offset_reg);
			assembler.move_reg_into_reg_pointer_8(value_reg, offset_reg);
			break;

		case 2:
			offset->get_value(assembler, offset_reg);
			assembler.multiply_8_into_reg(pointed_type.byte_size(), offset_reg);
			assembler.move_stack_top_address_into_reg(offset_reg);
			assembler.add_64_into_reg(location_data.offset, offset_reg);
			assembler.move_reg_into_reg_pointer_16(value_reg, offset_reg);
			break;

		case 4:
			offset->get_value(assembler, offset_reg);
			assembler.multiply_8_into_reg(pointed_type.byte_size(), offset_reg);
			assembler.move_stack_top_address_into_reg(offset_reg);
			assembler.add_64_into_reg(location_data.offset, offset_reg);
			assembler.move_reg_into_reg_pointer_32(value_reg, offset_reg);
			break;

		case 8:
			offset->get_value(assembler, offset_reg);
			assembler.multiply_8_into_reg(pointed_type.byte_size(), offset_reg);
			assembler.move_stack_top_address_into_reg(offset_reg);
			assembler.add_64_into_reg(location_data.offset, offset_reg);
			assembler.move_reg_into_reg_pointer_64(value_reg, offset_reg);
			break;

		default:
			err_at_token(pointer->accountable_token,
				"Type Error",
				"Variable doesn't fit in register\n"
				"Support for this is not implemented yet");
		}

		assembler.free_register(offset_reg);
	}

	void
	get_value(Assembler &assembler, uint8_t result_reg)
		const override
	{
		uint8_t offset_reg;

		Type pointed_type = type;

		// Multiply the offset by the byte size.

		offset_reg = assembler.get_register();
		offset->get_value(assembler, offset_reg);
		assembler.multiply_8_into_reg(pointed_type.byte_size(), offset_reg);

		// Add the offset into the pointer.

		pointer->get_value(assembler, result_reg);
		assembler.add_reg_into_reg(offset_reg, result_reg);

		assembler.free_register(offset_reg);

		// Dereference the pointer.

		switch (pointed_type.byte_size())
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

		default:
			printf("Dereference assignment for "
			       "	byte size %lu is not implemented\n",
				pointed_type.byte_size());
			abort();
		}
	}
};

#endif