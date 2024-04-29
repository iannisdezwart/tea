#ifndef TEA_AST_NODE_IDENTIFIER_EXPRESSION_HEADER
#define TEA_AST_NODE_IDENTIFIER_EXPRESSION_HEADER

#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/ASTNodes/WriteValue.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/code-gen/Assembler.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"
#include "Compiler/tokeniser.hpp"

struct IdentifierExpression final : public WriteValue
{
	IdentifierExpression(Token identifier_token)
		: WriteValue(std::move(identifier_token), IDENTIFIER_EXPRESSION) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
		override
	{
		callback(this, depth);
	}

	std::string
	to_str()
		override
	{
		std::string s = "IdentifierExpression { identifier = \""
			+ accountable_token.value + "\" } @ " + to_hex((size_t) this);
		return s;
	}

	void
	type_check(TypeCheckState &type_check_state)
		override
	{
		std::string id_name = accountable_token.value;

		type = type_check_state.get_type_of_identifier(id_name);

		if (type == Type::UNDEFINED)
		{
			err_at_token(accountable_token,
				"Identifier has unknown kind",
				"Identifier: %s. this might be a bug in the compiler",
				id_name.c_str());
		}

		IdentifierKind id_kind = type_check_state.get_identifier_kind(id_name);

		if (id_kind == IdentifierKind::UNDEFINED)
		{
			err_at_token(accountable_token,
				"Reference to undeclared variable",
				"Identifier %s was referenced, but not declared",
				id_name.c_str());
		}

		int64_t offset;
		uint64_t var_size;

		switch (id_kind)
		{
		case IdentifierKind::LOCAL:
		{
			VariableDefinition &var = type_check_state.locals[id_name];
			Type &type              = var.id.type;
			offset                  = var.offset;
			var_size                = type.byte_size();
			break;
		}

		case IdentifierKind::PARAMETER:
		{
			VariableDefinition &var = type_check_state.parameters[id_name];
			Type &type              = var.id.type;
			offset                  = -type_check_state.parameters_size + var.offset
				- 8 - CPU::stack_frame_size;
			var_size = type.byte_size();
			break;
		}

		case IdentifierKind::GLOBAL:
		{
			VariableDefinition &var = type_check_state.globals[id_name];
			Type &type              = var.id.type;
			offset                  = var.offset;
			var_size                = type.byte_size();
			break;
		}

		default:
			err_at_token(accountable_token,
				"Identifier has unknown kind",
				"Identifier: %s. this might be a bug in the compiler",
				id_name.c_str());
		}

		location_data = std::make_unique<LocationData>(id_kind, offset, var_size);
	}

	void
	get_value(Assembler &assembler, uint8_t result_reg)
		const override
	{
		// Local variable or parameter

		if (location_data->is_at_frame_top())
		{
			if (type.is_array())
			{
				assembler.move_reg_into_reg(R_FRAME_PTR, result_reg);
				assembler.add_64_into_reg(location_data->offset, result_reg);

				return;
			}

			switch (location_data->var_size)
			{
			case 1:
				assembler.move_frame_offset_8_into_reg(
					location_data->offset, result_reg);
				break;

			case 2:
				assembler.move_frame_offset_16_into_reg(
					location_data->offset, result_reg);
				break;

			case 4:
				assembler.move_frame_offset_32_into_reg(
					location_data->offset, result_reg);
				break;

			case 8:
				assembler.move_frame_offset_64_into_reg(
					location_data->offset, result_reg);
				break;

			default:
				err_at_token(accountable_token,
					"Type Error",
					"Variable doesn't fit in register\n"
					"Support for this is not implemented yet");
			}

			return;
		}

		// Global variable

		if (type.is_array())
		{
			assembler.move_stack_top_address_into_reg(result_reg);
			assembler.add_64_into_reg(location_data->offset, result_reg);

			return;
		}

		switch (location_data->var_size)
		{
		case 1:
			assembler.move_stack_top_offset_8_into_reg(
				location_data->offset, result_reg);
			break;

		case 2:
			assembler.move_stack_top_offset_16_into_reg(
				location_data->offset, result_reg);
			break;

		case 4:
			assembler.move_stack_top_offset_32_into_reg(
				location_data->offset, result_reg);
			break;

		case 8:
			assembler.move_stack_top_offset_64_into_reg(
				location_data->offset, result_reg);
			break;

		default:
			err_at_token(accountable_token,
				"Type Error",
				"Variable doesn't fit in register\n"
				"Support for this is not implemented yet");
		}
	}

	void
	store(Assembler &assembler, uint8_t value_reg)
		const override
	{
		// Local variable or parameter

		if (location_data->is_at_frame_top())
		{
			switch (location_data->var_size)
			{
			case 1:
				assembler.move_reg_into_frame_offset_8(
					value_reg, location_data->offset);
				break;

			case 2:
				assembler.move_reg_into_frame_offset_16(
					value_reg, location_data->offset);
				break;

			case 4:
				assembler.move_reg_into_frame_offset_32(
					value_reg, location_data->offset);
				break;

			case 8:
				assembler.move_reg_into_frame_offset_64(
					value_reg, location_data->offset);
				break;

			default:
				err_at_token(accountable_token,
					"Type Error",
					"Variable doesn't fit in register\n"
					"Support for this is not implemented yet");
			}

			return;
		}

		// Global variable

		switch (location_data->var_size)
		{
		case 1:
			assembler.move_reg_into_stack_top_offset_8(
				value_reg, location_data->offset);
			break;

		case 2:
			assembler.move_reg_into_stack_top_offset_16(
				value_reg, location_data->offset);
			break;

		case 4:
			assembler.move_reg_into_stack_top_offset_32(
				value_reg, location_data->offset);
			break;

		case 8:
			assembler.move_reg_into_stack_top_offset_64(
				value_reg, location_data->offset);
			break;

		default:
			err_at_token(accountable_token,
				"Type Error",
				"Variable doesn't fit in register\n"
				"Support for this is not implemented yet");
		}
	}
};

#endif