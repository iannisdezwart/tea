#ifndef TEA_AST_NODE_OFFSET_EXPRESSION_HEADER
#define TEA_AST_NODE_OFFSET_EXPRESSION_HEADER

#include "Compiler/ASTNodes/ReadValue.hpp"
#include "Compiler/ASTNodes/WriteValue.hpp"

struct OffsetExpression final : public WriteValue
{
	std::unique_ptr<WriteValue> pointer;
	std::unique_ptr<ReadValue> offset;

	OffsetExpression(CompactToken accountable_token,
		std::unique_ptr<WriteValue> pointer,
		std::unique_ptr<ReadValue> offset)
		: WriteValue(std::move(accountable_token), OFFSET_EXPRESSION),
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
		offset->get_value(assembler, offset_reg);

		uint8_t temp_reg = assembler.get_register();

		// Local variable or parameter

		if (location_data.is_at_frame_top())
		{
			switch (type.byte_size())
			{
			case 1:
				assembler.move_lit(pointed_type.byte_size(), temp_reg);
				assembler.mul_int_64(offset_reg, temp_reg);
				assembler.add_int_64(R_FRAME_PTR, offset_reg);
				assembler.move_lit(location_data.offset, temp_reg);
				assembler.add_int_64(temp_reg, offset_reg);
				assembler.store_ptr_8(value_reg, offset_reg);
				break;

			case 2:

				assembler.move_lit(pointed_type.byte_size(), temp_reg);
				assembler.mul_int_64(offset_reg, temp_reg);
				assembler.add_int_64(R_FRAME_PTR, offset_reg);
				assembler.move_lit(location_data.offset, temp_reg);
				assembler.add_int_64(temp_reg, offset_reg);
				assembler.store_ptr_16(value_reg, offset_reg);
				break;

			case 4:
				assembler.move_lit(pointed_type.byte_size(), temp_reg);
				assembler.mul_int_64(offset_reg, temp_reg);
				assembler.add_int_64(R_FRAME_PTR, offset_reg);
				assembler.move_lit(location_data.offset, temp_reg);
				assembler.add_int_64(temp_reg, offset_reg);
				assembler.store_ptr_32(value_reg, offset_reg);
				break;

			case 8:
				assembler.move_lit(pointed_type.byte_size(), temp_reg);
				assembler.mul_int_64(offset_reg, temp_reg);
				assembler.add_int_64(R_FRAME_PTR, offset_reg);
				assembler.move_lit(location_data.offset, temp_reg);
				assembler.add_int_64(temp_reg, offset_reg);
				assembler.store_ptr_64(value_reg, offset_reg);
				break;

			default:
				err_at_token(pointer->accountable_token,
					"Type Error",
					"Variable doesn't fit in register\n"
					"Support for this is not implemented yet");
			}
		}
		else
		{
			// Global variable

			switch (type.byte_size())
			{
			case 1:
				assembler.move_lit(pointed_type.byte_size(), temp_reg);
				assembler.mul_int_64(offset_reg, temp_reg);
				assembler.add_int_64(R_STACK_TOP_PTR, offset_reg);
				assembler.move_lit(location_data.offset, temp_reg);
				assembler.add_int_64(temp_reg, offset_reg);
				assembler.store_ptr_8(value_reg, offset_reg);
				break;

			case 2:
				assembler.move_lit(pointed_type.byte_size(), temp_reg);
				assembler.mul_int_64(offset_reg, temp_reg);
				assembler.add_int_64(R_STACK_TOP_PTR, offset_reg);
				assembler.move_lit(location_data.offset, temp_reg);
				assembler.add_int_64(temp_reg, offset_reg);
				assembler.store_ptr_16(value_reg, offset_reg);
				break;

			case 4:
				assembler.move_lit(pointed_type.byte_size(), temp_reg);
				assembler.mul_int_64(offset_reg, temp_reg);
				assembler.add_int_64(R_STACK_TOP_PTR, offset_reg);
				assembler.move_lit(location_data.offset, temp_reg);
				assembler.add_int_64(temp_reg, offset_reg);
				assembler.store_ptr_32(value_reg, offset_reg);
				break;

			case 8:
				assembler.move_lit(pointed_type.byte_size(), temp_reg);
				assembler.mul_int_64(offset_reg, temp_reg);
				assembler.add_int_64(R_STACK_TOP_PTR, offset_reg);
				assembler.move_lit(location_data.offset, temp_reg);
				assembler.add_int_64(temp_reg, offset_reg);
				assembler.store_ptr_64(value_reg, offset_reg);
				break;

			default:
				err_at_token(pointer->accountable_token,
					"Type Error",
					"Variable doesn't fit in register\n"
					"Support for this is not implemented yet");
			}
		}

		assembler.free_register(offset_reg);
		assembler.free_register(temp_reg);
	}

	void
	get_value(Assembler &assembler, uint8_t result_reg)
		const override
	{
		Type pointed_type = type;

		// Multiply the offset by the byte size.

		uint8_t offset_reg = assembler.get_register();
		uint8_t temp_reg   = assembler.get_register();
		offset->get_value(assembler, offset_reg);
		assembler.move_lit(pointed_type.byte_size(), temp_reg);
		assembler.mul_int_64(offset_reg, temp_reg);
		assembler.free_register(temp_reg);

		// Add the offset into the pointer.

		pointer->get_value(assembler, result_reg);
		assembler.add_int_64(offset_reg, result_reg);

		assembler.free_register(offset_reg);

		// Dereference the pointer.

		switch (pointed_type.byte_size())
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
			p_warn(stderr, "Dereference assignment for "
				       "	byte size %lu is not implemented\n",
				pointed_type.byte_size());
			abort();
		}
	}
};

constexpr int OFFSET_EXPRESSION_SIZE = sizeof(OffsetExpression);

#endif