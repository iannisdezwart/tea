#ifndef TEA_AST_NODE_IDENTIFIER_EXPRESSION_HEADER
#define TEA_AST_NODE_IDENTIFIER_EXPRESSION_HEADER

#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/ASTNodes/WriteValue.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/code-gen/Assembler.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"

struct IdentifierExpression final : public WriteValue
{
	std::string identifier;
	// Set during type checking if the identifier is a class field.
	std::optional<Type> object_type;

	IdentifierExpression(CompactToken accountable_token, std::string identifier)
		: WriteValue(std::move(accountable_token), IDENTIFIER_EXPRESSION),
		  identifier(identifier) {}

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
			+ identifier + "\" } @ " + to_hex((size_t) this);
		return s;
	}

	void
	type_check(TypeCheckState &type_check_state)
		override
	{
		if (object_type.has_value())
		{
			const ClassDefinition &class_def = type_check_state.classes.at(object_type->value);

			if (!class_def.has_field(identifier))
			{
				err_at_token(accountable_token,
					"Class field not found",
					"Field %s not found in class %d",
					identifier.c_str(), object_type->value);
			}

			type          = class_def.get_field_type(identifier);
			location_data = LocationData(IdentifierKind::UNDEFINED, class_def.get_field_offset(identifier));
			return;
		}

		type = type_check_state.get_type_of_identifier(identifier);

		if (type.value == UNDEFINED)
		{
			err_at_token(accountable_token,
				"Identifier has unknown kind",
				"Identifier: %s. this might be a bug in the compiler",
				identifier.c_str());
		}

		IdentifierKind id_kind = type_check_state.get_identifier_kind(identifier);

		if (id_kind == IdentifierKind::UNDEFINED)
		{
			err_at_token(accountable_token,
				"Reference to undeclared variable",
				"Identifier %s was referenced, but not declared",
				identifier.c_str());
		}

		int64_t offset;

		switch (id_kind)
		{
		case IdentifierKind::LOCAL:
		{
			const VariableDefinition &var = type_check_state.get_local(identifier);
			offset                        = var.offset;
			break;
		}

		case IdentifierKind::PARAMETER:
		{
			const VariableDefinition &var = type_check_state.parameters[identifier];
			offset                        = -type_check_state.parameters_size + var.offset
				- 8 - CPU::stack_frame_size;
			break;
		}

		case IdentifierKind::GLOBAL:
		{
			const VariableDefinition &var = type_check_state.globals[identifier];
			offset                        = var.offset;
			break;
		}

		default:
			err_at_token(accountable_token,
				"Identifier has unknown kind",
				"Identifier: %s. this might be a bug in the compiler",
				identifier.c_str());
		}

		location_data = LocationData(id_kind, offset);
	}

	void
	get_value(Assembler &assembler, uint8_t result_reg)
		const override
	{
		// Local variable or parameter

		if (location_data.is_at_frame_top())
		{
			if (type.is_array())
			{
				assembler.move_lit(location_data.offset, result_reg);
				assembler.add_int_64(R_FRAME_PTR, result_reg);

				return;
			}

			switch (type.byte_size())
			{
			case 1:
				assembler.move_lit(location_data.offset, result_reg);
				assembler.add_int_64(R_FRAME_PTR, result_reg);
				assembler.load_ptr_8(result_reg, result_reg);
				break;

			case 2:
				assembler.move_lit(location_data.offset, result_reg);
				assembler.add_int_64(R_FRAME_PTR, result_reg);
				assembler.load_ptr_16(result_reg, result_reg);
				break;

			case 4:
				assembler.move_lit(location_data.offset, result_reg);
				assembler.add_int_64(R_FRAME_PTR, result_reg);
				assembler.load_ptr_32(result_reg, result_reg);
				break;

			case 8:
				assembler.move_lit(location_data.offset, result_reg);
				assembler.add_int_64(R_FRAME_PTR, result_reg);
				assembler.load_ptr_64(result_reg, result_reg);
				break;

			default:
				assembler.move_lit(location_data.offset, result_reg);
				assembler.add_int_64(R_FRAME_PTR, result_reg);
				break;
			}

			return;
		}

		// Global variable

		if (type.is_array())
		{
			assembler.move_lit(location_data.offset, result_reg);
			assembler.add_int_64(R_STACK_TOP_PTR, result_reg);

			return;
		}

		switch (type.byte_size())
		{
		case 1:
			assembler.move_lit(location_data.offset, result_reg);
			assembler.add_int_64(R_STACK_TOP_PTR, result_reg);
			assembler.load_ptr_8(result_reg, result_reg);
			break;

		case 2:
			assembler.move_lit(location_data.offset, result_reg);
			assembler.add_int_64(R_STACK_TOP_PTR, result_reg);
			assembler.load_ptr_16(result_reg, result_reg);
			break;

		case 4:
			assembler.move_lit(location_data.offset, result_reg);
			assembler.add_int_64(R_STACK_TOP_PTR, result_reg);
			assembler.load_ptr_32(result_reg, result_reg);
			break;

		case 8:
			assembler.move_lit(location_data.offset, result_reg);
			assembler.add_int_64(R_STACK_TOP_PTR, result_reg);
			assembler.load_ptr_64(result_reg, result_reg);
			break;

		default:
			assembler.move_lit(location_data.offset, result_reg);
			assembler.add_int_64(R_STACK_TOP_PTR, result_reg);
			break;
		}
	}

	void
	store(Assembler &assembler, uint8_t value_reg)
		const override
	{
		// Local variable or parameter

		if (location_data.is_at_frame_top())
		{
			switch (type.byte_size())
			{
			case 1:
			{
				uint8_t dst_ptr_reg = assembler.get_register();
				assembler.move_lit(location_data.offset, dst_ptr_reg);
				assembler.add_int_64(R_FRAME_PTR, dst_ptr_reg);
				assembler.store_ptr_8(value_reg, dst_ptr_reg);
				assembler.free_register(dst_ptr_reg);
				break;
			}

			case 2:
			{
				uint8_t dst_ptr_reg = assembler.get_register();
				assembler.move_lit(location_data.offset, dst_ptr_reg);
				assembler.add_int_64(R_FRAME_PTR, dst_ptr_reg);
				assembler.store_ptr_16(value_reg, dst_ptr_reg);
				assembler.free_register(dst_ptr_reg);
				break;
			}

			case 4:
			{
				uint8_t dst_ptr_reg = assembler.get_register();
				assembler.move_lit(location_data.offset, dst_ptr_reg);
				assembler.add_int_64(R_FRAME_PTR, dst_ptr_reg);
				assembler.store_ptr_32(value_reg, dst_ptr_reg);
				assembler.free_register(dst_ptr_reg);
				break;
			}

			case 8:
			{
				uint8_t dst_ptr_reg = assembler.get_register();
				assembler.move_lit(location_data.offset, dst_ptr_reg);
				assembler.add_int_64(R_FRAME_PTR, dst_ptr_reg);
				assembler.store_ptr_64(value_reg, dst_ptr_reg);
				assembler.free_register(dst_ptr_reg);
				break;
			}

			default:
			{
				uint8_t dst_ptr_reg = assembler.get_register();
				assembler.move_lit(location_data.offset, dst_ptr_reg);
				assembler.add_int_64(R_FRAME_PTR, dst_ptr_reg);
				assembler.mem_copy(value_reg, dst_ptr_reg, type.byte_size());
				assembler.free_register(dst_ptr_reg);
				break;
			}
			}

			return;
		}

		// Global variable

		switch (type.byte_size())
		{
		case 1:
		{
			uint8_t dst_ptr_reg = assembler.get_register();
			assembler.move_lit(location_data.offset, dst_ptr_reg);
			assembler.add_int_64(R_STACK_TOP_PTR, dst_ptr_reg);
			assembler.store_ptr_8(value_reg, dst_ptr_reg);
			assembler.free_register(dst_ptr_reg);
			break;
		}

		case 2:
		{
			uint8_t dst_ptr_reg = assembler.get_register();
			assembler.move_lit(location_data.offset, dst_ptr_reg);
			assembler.add_int_64(R_STACK_TOP_PTR, dst_ptr_reg);
			assembler.store_ptr_16(value_reg, dst_ptr_reg);
			assembler.free_register(dst_ptr_reg);
			break;
		}

		case 4:
		{
			uint8_t dst_ptr_reg = assembler.get_register();
			assembler.move_lit(location_data.offset, dst_ptr_reg);
			assembler.add_int_64(R_STACK_TOP_PTR, dst_ptr_reg);
			assembler.store_ptr_32(value_reg, dst_ptr_reg);
			assembler.free_register(dst_ptr_reg);
			break;
		}

		case 8:
		{
			uint8_t dst_ptr_reg = assembler.get_register();
			assembler.move_lit(location_data.offset, dst_ptr_reg);
			assembler.add_int_64(R_STACK_TOP_PTR, dst_ptr_reg);
			assembler.store_ptr_64(value_reg, dst_ptr_reg);
			assembler.free_register(dst_ptr_reg);
			break;
		}

		default:
		{
			uint8_t dst_ptr_reg = assembler.get_register();
			assembler.move_lit(location_data.offset, dst_ptr_reg);
			assembler.add_int_64(R_STACK_TOP_PTR, dst_ptr_reg);
			assembler.mem_copy(value_reg, dst_ptr_reg, type.byte_size());
			assembler.free_register(dst_ptr_reg);
			break;
		}
		}
	}
};

constexpr int IDENTIFIER_EXPRESSION_SIZE = sizeof(IdentifierExpression);

#endif